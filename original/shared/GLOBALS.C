#define DOT		0
#define WIRE	1
#define SOLID	2

#define Black   16
#define White   31
#define Grey    16
#define	Red     32
#define	Green   48
#define	Blue    64
#define	Yellow  80
#define	Cyan    96
#define	Magenta 112
#define	Orange	128
#define Mint	144
#define Pink	160
#define SkyBlue	176
#define Purple	192
#define Teal	208
#define Poo		224


float s[361],c[361];


struct Object {
	int *x,*y,*z;
	int *poly1,*poly2,*poly3;
	char *color;

	int points,polys;
	int max_points,max_polys;
};



struct {
	int *x,*y,*z;
	int *sx,*sy;

	int max_points;

	int xa,ya,za;
	int xp,yp,zp;

	char pers;
} Arena;