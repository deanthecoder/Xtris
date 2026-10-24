import { VGA } from './vga.js';
import { SHAPES, COLORS, BLOCK_MASK } from './game-data.js';

const canvas = document.querySelector('#screen');
const status = document.querySelector('#status');
const vga = new VGA(canvas);
const LEFT = 99, TOP = 9, COLUMNS = 12, ROWS = 18;
const emptyBoard = () => Array.from({ length: ROWS }, () => Array(COLUMNS).fill(0));

class Xtris {
  constructor() {
    this.phase = 'loading';
    this.lastFrame = 0;
    this.wave = 0;
    this.snow = [];
    this.snowLayer = new Uint8Array(320 * 200);
    this.fame = this.loadFame();
    this.audio = {};
    this.audioContext = null;
    this.introStart = 0;
  }

  async init() {
    await Promise.all([
      vga.loadSprite('AREA.SPR'), vga.loadSprite('SNOWMAN.SPR'),
      vga.loadSprite('BALLS.SPR'), vga.loadPCX('intro.ils'),
      vga.loadFont('FONTS.DAT'),
    ]);
    this.bindInput();
    this.prepareAudio();
    this.phase = 'intro';
    this.introStart = performance.now();
    status.textContent = 'Press any key to start';
    requestAnimationFrame(time => this.frame(time));
  }

  bindInput() {
    const fit = () => {
      const wrap = document.querySelector('.screen-wrap');
      const toolbar = document.querySelector('.toolbar');
      const aspect = document.querySelector('#crt-aspect').checked ? 4 / 3 : 8 / 5;
      const availableHeight = Math.max(180, innerHeight - wrap.getBoundingClientRect().top - toolbar.offsetHeight - 24);
      const width = Math.floor(Math.min(1280, wrap.clientWidth, availableHeight * aspect));
      canvas.style.width = `${width}px`;
      canvas.style.height = `${Math.floor(width / aspect)}px`;
    };
    addEventListener('resize', fit);
    document.querySelector('#crt-aspect').addEventListener('change', fit);
    fit();
    document.querySelector('#new-game').addEventListener('click', () => { this.unlockAudio(); this.newGame(); canvas.focus(); });
    document.querySelector('#pause').addEventListener('click', () => { this.togglePause(); canvas.focus(); });
    canvas.addEventListener('pointerdown', () => { this.unlockAudio(); canvas.focus(); if (this.phase === 'intro') this.newGame(); });
    addEventListener('keydown', event => {
      const keys = ['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'Escape', ' '];
      if (keys.includes(event.key)) event.preventDefault();
      this.unlockAudio();
      this.key(event.key);
    });
  }

  async prepareAudio() {
    const Context = window.AudioContext || window.webkitAudioContext;
    if (!Context) return;
    try {
      this.audioContext = new Context();
      const files = { rotate: 'DINK.snd', drop: 'FALL.snd', line: 'LINE.snd',
        high: 'Woo.snd', sheep: 'Sheep.snd' };
      await Promise.all(Object.entries(files).map(async ([key, file]) => {
        const response = await fetch(`assets/${file}`);
        if (response.ok) this.audio[key] = await this.audioContext.decodeAudioData(await response.arrayBuffer());
      }));
    } catch (error) { console.warn('Sound unavailable:', error); }
  }
  unlockAudio() { if (this.audioContext?.state === 'suspended') this.audioContext.resume(); }
  sound(name) {
    if (!document.querySelector('#sound').checked || !this.audioContext || !this.audio[name]) return;
    const source = this.audioContext.createBufferSource();
    source.buffer = this.audio[name]; source.connect(this.audioContext.destination); source.start();
  }

  loadFame() {
    const fallback = Array.from({ length: 10 }, (_, i) => ({ name: 'DEAN EDIS', score: (10 - i) * 1000 }));
    try {
      const saved = JSON.parse(localStorage.getItem('xtris-fame'));
      if (Array.isArray(saved) && saved.length === 10 &&
          saved.every(row => typeof row.name === 'string' && Number.isInteger(row.score))) return saved;
    } catch (_) { /* Private browsing may disable local storage. */ }
    return fallback;
  }
  saveFame() { try { localStorage.setItem('xtris-fame', JSON.stringify(this.fame)); } catch (_) {} }

  resetSnow() {
    this.snowLayer.fill(0);
    this.snow = Array.from({ length: 70 }, () => ({
      x: Math.floor(Math.random() * 320), y: Math.floor(Math.random() * 190),
      speed: Math.random() < 0.5 ? 1 : 2,
    }));
    this.snowStep = 0;
  }
  inPlayFrame(x, y) { return x > LEFT - 7 && x < LEFT + 127 && y > TOP - 7 && y < TOP + 187; }
  updateSnow(dt) {
    this.snowStep += dt * 45;
    while (this.snowStep >= 1) {
      this.snowStep--;
      for (const flake of this.snow) {
        if (Math.random() > 1 / flake.speed) continue;
        let nextY = flake.y + 1;
        if (nextY >= 199 || this.snowLayer[nextY * 320 + flake.x]) {
          if (!this.inPlayFrame(flake.x, flake.y)) this.snowLayer[flake.y * 320 + flake.x] = 31;
          flake.x = Math.floor(Math.random() * 320);
          flake.y = 0;
        } else flake.y = nextY;
      }
    }
  }

  newGame() {
    this.board = emptyBoard();
    this.score = 0;
    this.difficulty = 1;
    this.fall = 0;
    this.current = null;
    this.resetSnow();
    this.phase = 'playing';
    status.textContent = 'Score 000000';
    this.spawn();
  }
  spawn() {
    const index = Math.floor(Math.random() * SHAPES.length);
    this.current = { grid: SHAPES[index].slice(), color: COLORS[index], x: 4, y: 0 };
    this.difficulty += 0.004;
    this.fall = 0;
    if (this.overlap(this.current.x, this.current.y, this.current.grid)) this.finish();
  }
  overlap(x, y, grid) {
    for (let row = 0; row < 4; row++) for (let col = 0; col < 4; col++) {
      if (!grid[row * 4 + col]) continue;
      const bx = x + col, by = y + row;
      if (bx < 0 || bx >= COLUMNS || by >= ROWS || (by >= 0 && this.board[by][bx])) return true;
    }
    return false;
  }
  move(dx) {
    if (this.phase !== 'playing' || !this.current) return;
    const next = this.current.x + dx;
    if (!this.overlap(next, this.current.y, this.current.grid)) this.current.x = next;
  }
  rotate() {
    if (this.phase !== 'playing' || !this.current) return;
    const old = this.current.grid, rotated = [];
    for (let col = 3; col >= 0; col--) for (let row = 0; row < 4; row++) rotated.push(old[row * 4 + col]);
    // The source shifts empty top rows up and clears the last row.
    // Rebuild from the rotated shape so all four rows remain in place.
    let clean = rotated;
    while (!clean.slice(0, 4).some(Boolean)) clean = clean.slice(4).concat([0, 0, 0, 0]);
    if (!this.overlap(this.current.x, this.current.y, clean)) { this.current.grid = clean; this.sound('rotate'); }
  }
  down(natural = true) {
    if (this.phase !== 'playing' || !this.current) return false;
    if (!this.overlap(this.current.x, this.current.y + 1, this.current.grid)) {
      this.current.y++; return true;
    }
    if (natural) this.score = (this.score + 10) & 0xffff;
    this.lock();
    return false;
  }
  hardDrop() {
    if (this.phase !== 'playing' || !this.current) return;
    this.sound('drop');
    while (!this.overlap(this.current.x, this.current.y + 1, this.current.grid)) {
      this.current.y++;
      this.score = (this.score + 2) & 0xffff;
    }
    this.lock();
  }
  lock() {
    const piece = this.current;
    for (let row = 0; row < 4; row++) for (let col = 0; col < 4; col++)
      if (piece.grid[row * 4 + col]) this.board[piece.y + row][piece.x + col] = piece.color;
    let bonus = 250;
    for (let row = ROWS - 1; row > 0;) {
      if (this.board[row].every(Boolean)) {
        this.sound('line');
        this.board.splice(row, 1);
        this.board.unshift(Array(COLUMNS).fill(0));
        this.score = (this.score + bonus) & 0xffff;
        bonus *= 2;
        this.difficulty += 0.1;
        row = ROWS - 1;
      } else row--;
    }
    status.textContent = `Score ${String(this.score).padStart(6, '0')}`;
    this.spawn();
  }
  finish() {
    const rank = this.fame.findIndex(row => this.score >= row.score);
    this.rank = rank;
    if (rank >= 0) {
      this.fame.splice(rank, 0, { name: '', score: this.score });
      this.fame.length = 10;
      this.name = '';
      this.phase = 'name';
      this.sound('high');
      status.textContent = 'New high score — enter your name';
    } else {
      this.phase = 'hall';
      this.sound('sheep');
      status.textContent = 'Game over — Y for another game, N to stop';
    }
  }
  togglePause() {
    if (this.phase === 'playing') { this.phase = 'paused'; status.textContent = 'Paused'; }
    else if (this.phase === 'paused') { this.phase = 'playing'; status.textContent = `Score ${String(this.score).padStart(6, '0')}`; }
  }
  key(key) {
    if (this.phase === 'intro') { this.newGame(); return; }
    if (this.phase === 'paused') { if (key.toLowerCase() === 'p' || key === 'Escape') this.togglePause(); return; }
    if (this.phase === 'confirm') {
      if (key.toLowerCase() === 'y') this.finish();
      if (key.toLowerCase() === 'n' || key === 'Escape') { this.phase = 'playing'; status.textContent = `Score ${String(this.score).padStart(6, '0')}`; }
      return;
    }
    if (this.phase === 'name') {
      if (key === 'Enter') {
        this.fame[this.rank].name = this.name.trim() || 'NONAME';
        this.saveFame(); this.phase = 'hall';
        status.textContent = 'Game over — Y for another game, N to stop';
      } else if (key === 'Backspace') this.name = this.name.slice(0, -1);
      else if (/^[a-z0-9 .!?'-]$/i.test(key) && this.name.length < 12) this.name += key.toUpperCase();
      return;
    }
    if (this.phase === 'hall' || this.phase === 'stopped') {
      if (key.toLowerCase() === 'y' || key === 'Enter') this.newGame();
      if (key.toLowerCase() === 'n') { this.phase = 'stopped'; status.textContent = 'Stopped — use New game to play again'; }
      return;
    }
    if (this.phase !== 'playing') return;
    if (key === 'ArrowLeft') this.move(-1);
    else if (key === 'ArrowRight') this.move(1);
    else if (key === 'ArrowUp') this.rotate();
    else if (key === 'ArrowDown' || key === ' ') this.hardDrop();
    else if (key === 'Escape') { this.phase = 'confirm'; status.textContent = 'Quit game? Y / N'; }
    else if (key.toLowerCase() === 'p') this.togglePause();
  }

  block(x, y, color) {
    for (let dy = 0; dy < 10; dy++) for (let dx = 0; dx < 10; dx++)
      vga.pixel(x + dx, y + dy, color + BLOCK_MASK[dy * 10 + dx]);
  }
  drawSnow() {
    for (let i = 0; i < this.snowLayer.length; i++) if (this.snowLayer[i]) vga.pixels[i] = this.snowLayer[i];
    for (const flake of this.snow) if (!this.inPlayFrame(flake.x, flake.y)) vga.pixel(flake.x, flake.y, 31);
  }
  drawScore() {
    const label = `Score - ${String(this.score).padStart(6, '0')}`;
    this.wave = (this.wave + 1) % 2880;
    for (let ch = 13; ch >= 0; ch--) {
      const y = Math.round(5 * Math.sin((this.wave >> 3) + ch * 12.8));
      vga.text(label[ch], 10 + ch * 5, 7 + y, Math.max(144, Math.min(159, 154 + y)));
    }
  }
  drawPlay() {
    vga.setPalette(vga.sprites['AREA.SPR'].palette);
    vga.clear(0);
    vga.sprite(vga.sprites['AREA.SPR'], 93, 3);
    vga.sprite(vga.sprites['BALLS.SPR'], 20, 167);
    vga.sprite(vga.sprites['SNOWMAN.SPR'], 235, 137);
    vga.fill(0, 199, 319, 199, 31);
    this.drawSnow();
    for (let row = 0; row < ROWS; row++) for (let col = 0; col < COLUMNS; col++)
      if (this.board[row][col]) this.block(LEFT + col * 10, TOP + row * 10, this.board[row][col]);
    if (this.current && ['playing', 'paused', 'confirm'].includes(this.phase)) {
      const { x, y, grid, color } = this.current;
      for (let row = 0; row < 4; row++) for (let col = 0; col < 4; col++)
        if (grid[row * 4 + col]) this.block(LEFT + (x + col) * 10, TOP + (y + row) * 10, color);
    }
    this.drawScore();
    if (this.phase === 'paused') this.overlay('PAUSED', 'PRESS P TO CONTINUE');
    if (this.phase === 'confirm') this.overlay('ARE YOU SURE?', 'Y YES   N NO');
  }
  overlay(title, instruction) {
    vga.fill(102, 83, 219, 114, 16);
    vga.fill(102, 83, 219, 83, 191);
    vga.fill(102, 114, 219, 114, 191);
    vga.text(title, 122, 89, 191);
    vga.text(instruction, 115, 102, 31);
  }
  drawHall() {
    this.drawPlay();
    vga.fill(LEFT, TOP, LEFT + 121, TOP + 181, 16);
    vga.text('HALL OF FAME', 134, 30, 223);
    for (let i = 0; i < 10; i++) {
      const y = 50 + i * 6, row = this.fame[i];
      if (i === this.rank && this.phase === 'name') vga.fill(LEFT, y, LEFT + 121, y + 4, 69);
      vga.text(i === this.rank && this.phase === 'name' ? this.name + '_' : row.name,
        LEFT + 3, y, i === this.rank && this.phase === 'name' ? 31 : 142 - i);
      vga.text(String(row.score).padStart(6, '0'), LEFT + 95, y,
        i === this.rank && this.phase === 'name' ? 31 : 142 - i);
    }
    if (this.phase === 'name') vga.text('ENTER NAME', 125, 132, 31);
    else {
      vga.text('ANOTHER GO?', 137, 180, 191);
      vga.text('Y / N', 148, 189, 31);
    }
  }
  drawIntro(time) {
    const picture = vga.pictures['intro.ils'];
    vga.setPalette(picture.palette); vga.picture(picture);
    const elapsed = (time - this.introStart) / 1000;
    if (elapsed >= 4.4) { this.newGame(); return; }
    const brightness = elapsed < 0.8 ? elapsed / 0.8 : elapsed > 3.9 ? (4.4 - elapsed) / 0.5 : 1;
    vga.present(Math.max(0, Math.min(1, brightness)));
  }
  frame(time) {
    const dt = Math.min(0.1, (time - (this.lastFrame || time)) / 1000);
    this.lastFrame = time;
    if (this.phase === 'intro') this.drawIntro(time);
    else if (this.board) {
      if (this.phase === 'playing') {
        this.fall += dt * this.difficulty;
        if (this.fall >= 1) { this.fall -= 1; this.down(); }
      }
      if (this.phase !== 'paused') this.updateSnow(dt);
      if (this.phase === 'name' || this.phase === 'hall' || this.phase === 'stopped') this.drawHall();
      else this.drawPlay();
      vga.present();
    }
    requestAnimationFrame(next => this.frame(next));
  }
}

const game = new Xtris();
game.init().catch(error => { console.error(error); status.textContent = `Could not start: ${error.message}`; });
