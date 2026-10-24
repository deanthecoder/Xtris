// Indexed-color Mode 13h-style framebuffer and readers for the archived art.
export class VGA {
  constructor(canvas) {
    this.ctx = canvas.getContext('2d', { alpha: false });
    this.ctx.imageSmoothingEnabled = false;
    this.pixels = new Uint8Array(320 * 200);
    this.palette = new Uint8Array(768);
    this.image = this.ctx.createImageData(320, 200);
    this.sprites = {};
    this.pictures = {};
  }

  async loadSprite(name) {
    const response = await fetch(`assets/${name}`);
    if (!response.ok) throw new Error(`Could not load ${name}`);
    const bytes = new Uint8Array(await response.arrayBuffer());
    if (String.fromCharCode(...bytes.slice(0, 3)) !== 'SPR' ||
        String.fromCharCode(...bytes.slice(8, 12)) !== 'DATA')
      throw new Error(`${name} is not an original SPR file`);
    const width = bytes[3] | bytes[4] << 8;
    const height = bytes[5] | bytes[6] << 8;
    if (bytes.length !== 780 + width * height) throw new Error(`${name} has the wrong sprite length`);
    return this.sprites[name] = {
      width, height, transparent: bytes[7],
      palette: bytes.slice(12, 780), pixels: bytes.slice(780),
    };
  }

  async loadPCX(name) {
    const response = await fetch(`assets/${name}`);
    if (!response.ok) throw new Error(`Could not load ${name}`);
    const bytes = new Uint8Array(await response.arrayBuffer());
    if (bytes[0] !== 10 || bytes[2] !== 1 || bytes[3] !== 8 || bytes[65] !== 1)
      throw new Error(`${name} is not an 8-bit PCX image`);
    const u16 = at => bytes[at] | bytes[at + 1] << 8;
    const width = u16(8) - u16(4) + 1, height = u16(10) - u16(6) + 1;
    const stride = u16(66), pixels = new Uint8Array(width * height);
    let at = 128;
    for (let y = 0; y < height; y++) {
      let x = 0;
      while (x < stride) {
        let value = bytes[at++], count = 1;
        if ((value & 0xc0) === 0xc0) { count = value & 0x3f; value = bytes[at++]; }
        while (count-- > 0) { if (x < width) pixels[y * width + x] = value; x++; }
      }
    }
    const palette = bytes.slice(-768).map(value => value >> 2);
    return this.pictures[name] = { width, height, pixels, palette };
  }

  async loadFont(name) {
    const response = await fetch(`assets/${name}`);
    if (!response.ok) throw new Error(`Could not load ${name}`);
    const source = await response.text();
    const parse = label => {
      const match = source.match(new RegExp(`${label}\\[\\]=\\{([\\s\\S]*?)\\};`));
      if (!match) throw new Error(`Could not read ${label} from ${name}`);
      return [...match[1].matchAll(/\d+/g)].map(entry => Number(entry[0]));
    };
    this.fontWidths = parse('Textw');
    this.fontBits = parse('VGATextInfo');
    this.fontOffsets = [];
    let offset = 0;
    for (const width of this.fontWidths) { this.fontOffsets.push(offset); offset += width * 6; }
  }

  clear(color = 0) { this.pixels.fill(color); }
  setPalette(palette) { this.palette.set(palette); }
  pixel(x, y, color) {
    if (x >= 0 && x < 320 && y >= 0 && y < 200) this.pixels[y * 320 + x] = color;
  }
  fill(x1, y1, x2, y2, color) {
    const left = Math.max(0, Math.min(x1, x2));
    const right = Math.min(319, Math.max(x1, x2));
    for (let y = Math.max(0, Math.min(y1, y2)); y <= Math.min(199, Math.max(y1, y2)); y++)
      this.pixels.fill(color, y * 320 + left, y * 320 + right + 1);
  }
  sprite(sprite, x, y) {
    for (let sy = 0; sy < sprite.height; sy++) {
      const dy = y + sy;
      if (dy < 0 || dy >= 200) continue;
      for (let sx = 0; sx < sprite.width; sx++) {
        const dx = x + sx, value = sprite.pixels[sy * sprite.width + sx];
        if (dx >= 0 && dx < 320 && value !== sprite.transparent) this.pixels[dy * 320 + dx] = value;
      }
    }
  }
  picture(picture) { this.pixels.set(picture.pixels); }
  text(value, x, y, color) {
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=!"£$%^&*()_+,.?\'';
    for (const letter of String(value).toUpperCase()) {
      if (letter === ' ') { x += 5; continue; }
      const index = chars.indexOf(letter);
      if (index < 0 || index >= this.fontWidths.length) { x += 4; continue; }
      const width = this.fontWidths[index], offset = this.fontOffsets[index];
      for (let fy = 0; fy < 6; fy++) for (let fx = 0; fx < width; fx++)
        if (this.fontBits[offset + fy * width + fx]) this.pixel(x + fx, y + fy, color);
      x += width + 1;
    }
  }
  present(brightness = 1) {
    const out = this.image.data;
    for (let i = 0; i < this.pixels.length; i++) {
      const index = this.pixels[i] * 3, p = i * 4;
      out[p] = Math.round(this.palette[index] * 255 / 63 * brightness);
      out[p + 1] = Math.round(this.palette[index + 1] * 255 / 63 * brightness);
      out[p + 2] = Math.round(this.palette[index + 2] * 255 / 63 * brightness);
      out[p + 3] = 255;
    }
    this.ctx.putImageData(this.image, 0, 0);
  }
}
