#define FLAKES	70

// Snowflake structure
struct {
	int x,y,vx,off;
	int under[3];
	int amplitude,speed;
} flake[FLAKES];



// Drop Snow
void DrawSnow(void) {
	// Variables
	int p,cond;
	static CountDracula=0;

	// Erase score area
	FillBox(10,3,left-10,18,Black);

	// Erase
	for(p=0;p<FLAKES;p++)
		if(!(flake[p].vx>left-7 && flake[p].vx<left+127 &&
		   flake[p].y>top-7 && flake[p].y<top+187))
			PutPixel(flake[p].vx,flake[p].y,Black);

	// Drop flakes (Urrghh!)
	CountDracula++;
	CountDracula%=2;
	for(p=0;p<FLAKES;p++) {
		if(flake[p].speed<=CountDracula) {
			flake[p].vx=flake[p].x;

			cond=0;
			if(!(GetPixel(flake[p].vx,flake[p].y+1)<17)) {
				if(GetPixel(flake[p].vx-1,flake[p].y+1)<17)
					cond=1;
				if(GetPixel(flake[p].vx+1,flake[p].y+1)<17)
					cond=2;
				if(!cond)
					cond=3;
			}
			if(flake[p].vx<0 || flake[p].vx>319)
				cond=3;

			switch(cond) {
				case 0 : // Drop
						 flake[p].y++;
						 break;
				case 1 : // Move left
						 flake[p].x--;
						 break;
				case 2 : // Move right
						 flake[p].x++;
						 break;
				case 3 : // Re-drop
						 if(!(flake[p].vx>left-7 && flake[p].vx<left+127 &&
							flake[p].y>top-7 && flake[p].y<top+187))
							PutPixel(flake[p].vx,flake[p].y,White);

						 // Make a new flake
						 do {
							 flake[p].x=random(320);
						 } while(GetPixel(flake[p].x,0)>=17);
						 flake[p].y=0;
						 flake[p].speed=random(2);

						 break;
			}
		}
	}

	// Draw
	for(p=0;p<FLAKES;p++)
		if(!(flake[p].vx>left-7 && flake[p].vx<left+127 &&
		   flake[p].y>top-7 && flake[p].y<top+187))
			PutPixel(flake[p].vx,flake[p].y,White);
}