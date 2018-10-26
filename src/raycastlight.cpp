#include "lib/retro.h"
#include "lib/retromain.h"
#include "raycast.h"

#define WALL_HEIGHT 64
#define VIEWER_DISTANCE 192
#define VIEWPORT_LEFT 0
#define VIEWPORT_RIGHT (RETRO_WIDTH)
#define VIEWPORT_TOP 0
#define VIEWPORT_BOT (RETRO_HEIGHT)
#define VIEWPORT_CENTER (VIEWPORT_TOP + (VIEWPORT_BOT - VIEWPORT_TOP) / 2)
#define TEXTURE_WIDTH 320
#define IMAGE_HEIGHT 64
#define IMAGE_WIDTH 64
#define FINE_GRIDSIZE 64
#define GRIDSIZE 16
#define MAXDISTANCE (64 * GRIDSIZE)
#define MAXLIGHT 32
#define MULTIPLIER 3

unsigned char litetable[MAXLIGHT + 1][RETRO_COLORS];
unsigned char litelevel[MAXDISTANCE];

void InitLightTables(Palette *palette)
{
	int red[RETRO_COLORS], green[RETRO_COLORS], blue[RETRO_COLORS];

	int redtarget = 0;
	int greentarget = 0;
	int bluetarget = 0;

	// Copy palette to RGB arrays
	for (int color = 0; color < RETRO_COLORS; color++)
	{
		red[color] = palette->r;
		green[color] = palette->g;
		blue[color] = palette->b;
		palette++;
	}

	// Iterate through all 256 palette colors
	for (int color = 0; color < RETRO_COLORS; color++) {
		// Iterate through all 32 + 1 intensity levels
		for (int level = 0; level <= MAXLIGHT; level++) {
			// Calculate ideal lightsourced RGB values
			float redlite = (float) (red[color] - redtarget) / MAXLIGHT * level + redtarget;
			float greenlite = (float) (green[color] - greentarget) / MAXLIGHT * level + greentarget;
			float bluelite = (float) (blue[color] - bluetarget) / MAXLIGHT * level + bluetarget;

			// Initialize score to very large number
			float bestscore = RETRO_COLORS*3;

			// Search the palette for closest match
			for (int color2 = 0; color2 < RETRO_COLORS; color2++) {
				// Assign proximity score to color
				float redscore = fabs(red[color2] - redlite);
				float greenscore = fabs(green[color2] - greenlite);
				float bluescore = fabs(blue[color2] - bluelite);
				float score = redscore + greenscore + bluescore;

				// Is this score better (i.e. smaller) than previous best?
				if (score < bestscore) {
					// If so, replace best score
					bestscore = score;

					// Remember which color got it
					litetable[level][color] = color2;
				}
			}
		}
	}

	// Calculate light intensities for all possible distances
	for (int distance = 1; distance < MAXDISTANCE; distance++) {
		float ratio = (float) MAXLIGHT / distance * MULTIPLIER;
		if (ratio > 1.0) {
			ratio = 1.0;
		}
		litelevel[distance] = ratio * MAXLIGHT;
	}
}

// Draws a raycast image in the viewport of the maze represented in array
// map as seen from position xview, yview by a viewer looking at an angle
// viewer_angle, where angle 0 is due north. (Angles measured in radians)
void draw_maze(unsigned char *screen, int xview, int yview, float viewing_angle, int viewer_height, unsigned char *image, map_type map, map_type flor, map_type ceiling,
	map_type flor_lites, map_type ceiling_lites, int ambient_level, unsigned char litesource[MAXLIGHT + 1][RETRO_COLORS], unsigned char* litelevel)
{
	int offset;                 // Pixel y position and offset
	float xd, yd;               // Distance to next wall in x and y
	int grid_x, grid_y;         // Coordinates of x and y grid lines
	float xcross_x, xcross_y;   // Ray intersection coordinates
	float ycross_x, ycross_y;
	unsigned int xdist, ydist;  // Distance to x and y grid lines
	int xmaze, ymaze;           // Map location of ray collision
	int distance;               // Distance to wall along ray
	int tmcolumn;               // Column in texture map
	float yratio;

	// Loop through all columns of pixels in viewport
	for (int column = VIEWPORT_LEFT; column < VIEWPORT_RIGHT; column++) {
		// Calculate horizontal angle of ray relative to center ray
		float column_angle = atan((float) (column - RETRO_WIDTH/2) / VIEWER_DISTANCE);

		// Calculate angle of ray relative to maze coordinates
		float radians = viewing_angle + column_angle;

		// Rotate endpoint of ray to viewing angle
		int x2 = -1024 * (sin(radians));
		int y2 = 1024 * (cos(radians));

		// Translate relative to viewer's position
		x2 += xview;
		y2 += yview;

		// Initialize ray at viewer's position
		float x = xview;
		float y = yview;

		// Find difference in x,y coordates along the ray
		int xdiff = x2 - xview;
		int ydiff = y2 - yview;

		// Cheat to avoid divide-by-zero error
		if (xdiff == 0) {
			xdiff = 1;
		}

		// Get slope of ray
		float slope = (float) ydiff / xdiff;
		if (slope == 0.0) {
			slope = 0.0001;
		}

		// Cast ray from grid line to grid line
		for (;;) {
			// If ray direction positive in x, get next x grid line, else get last x grid line
			if (xdiff > 0) {
				grid_x = ((int)x & 0xffc0) + FINE_GRIDSIZE;
			} else {
				grid_x = ((int)x & 0xffc0) - 1;
			}

			// If ray direction positive in y, get next y grid line, else get last y grid line
			if (ydiff > 0) {
				grid_y = ((int)y & 0xffc0) + FINE_GRIDSIZE;
			} else {
				grid_y = ((int)y & 0xffc0) - 1;
			}

			// Get y,y coordinates where ray crosses x grid line
			xcross_x = grid_x;
			xcross_y = y + slope * (grid_x - x);

			// Get x,y coordinates where ray crosses y grid line
			ycross_x = x + (grid_y - y) / slope;
			ycross_y = grid_y;

			// Get distance to x grid line
			xd = xcross_x - x;
			yd = xcross_y - y;
			xdist = sqrt(xd * xd + yd * yd);

			// Get distance to y grid line
			xd = ycross_x - x;
			yd = ycross_y - y;
			ydist = sqrt(xd * xd + yd * yd);

			// If x grid line is closer ...
			if (xdist < ydist) {
				// Set x and y to point of ray intersection
				x = xcross_x;
				y = xcross_y;

				// Find relevant column of texture map
				tmcolumn = (int) y % FINE_GRIDSIZE;
			} else {
				// if y grid line is closer

				// Set x and y to piont of ray intersection
				x = ycross_x;
				y = ycross_y;

				// Find relevant column of texture map
				tmcolumn = (int) x % FINE_GRIDSIZE;
			}

			// Calculate maze grid coordinates of square
			xmaze = x / FINE_GRIDSIZE;
			ymaze = y / FINE_GRIDSIZE;

			// Is there a maze cube here? If so, stop looping
			if (map[xmaze][ymaze]) {
				break;
			}
		}

		// Get distance from viewer to intersection point
		xd = x - xview;
		yd = y - yview;
		distance = (long) sqrt(xd * xd + yd * yd) * cos(column_angle);
		if (distance == 0) {
			distance = 1;
		}

		// Calculate visible height of walls
		int height = VIEWER_DISTANCE * WALL_HEIGHT / distance;

		// Calculate bottom of wall on screen
		int bot = VIEWER_DISTANCE * viewer_height / distance + VIEWPORT_CENTER;

		// Calculate top of wall on screen
		int top = bot - height + 1;

		// If top of current vertical line is outside of viewport, clip it
		int iheight = IMAGE_HEIGHT;
		yratio = (float) WALL_HEIGHT / height;
		if (top < VIEWPORT_TOP) {
			tmcolumn += (int) ((VIEWPORT_TOP-top) * yratio) * TEXTURE_WIDTH;
			iheight -= ((VIEWPORT_TOP - top) * yratio);
			top = VIEWPORT_TOP;
		}
		if (bot > VIEWPORT_BOT) {
			iheight -= (bot - VIEWPORT_BOT) * yratio;
			bot = VIEWPORT_BOT;
		}

		// Point to video memory offset for top of line
		offset = top * RETRO_WIDTH + column;

		// Initialize vertical error term for texture map
		int tyerror = IMAGE_HEIGHT;

		// Which graphics tile are we using?
		int tile = map[xmaze][ymaze] - 1;

		// Find offset of tile and column in bitmap
		unsigned int tileptr = (tile / 5) * TEXTURE_WIDTH * IMAGE_HEIGHT + (tile % 5) * IMAGE_WIDTH + tmcolumn;

		// Loop through all pixels in the current vertical line, advancing offset to the next row of pixels after each pixel is drawn
		for (int h = 0; h < iheight; h++) {
			// Are we ready to draw a pixel?
			while (tyerror >= IMAGE_HEIGHT) {
				// If so, draw it
				int level = litelevel[distance] + ambient_level + flor_lites[xmaze][ymaze];
				if (level > MAXLIGHT) {
					level = MAXLIGHT;
				}
				screen[offset] = litesource[level][image[tileptr]];

				// Reset error term
				tyerror -= IMAGE_HEIGHT;

				// Advance offset to next screen line
				offset += RETRO_WIDTH;
			}

			// Incremental division
			tyerror += height;

			// Advance tileptr to next line of bitmap
			tileptr += TEXTURE_WIDTH;
		}

		// Step through floor pixels
		for (int row = bot + 1; row < VIEWPORT_BOT; row++) {
			// Get ratio of viewer's height to pixel height
			float ratio = (float) viewer_height / (row - VIEWPORT_CENTER);

			// Get distance to visible pixel
			distance = ratio * VIEWER_DISTANCE / cos(column_angle);

			// Rotate distance to ray angle
			int x = - distance * (sin(radians));
			int y = distance * (cos(radians));

			// Translate relative to viewer coordinates
			x += xview;
			y += yview;

			// Get maze square intersected by ray
			int xmaze = x / FINE_GRIDSIZE;
			int ymaze = y / FINE_GRIDSIZE;

			// Find relevant column of texture map
			int t = (y % FINE_GRIDSIZE) * TEXTURE_WIDTH + (x % FINE_GRIDSIZE);

			// Which graphics tile are we using?
			int tile = flor[xmaze][ymaze];

			// Find offset of tile and column in bitmap
			unsigned int tileptr = (tile / 5) * TEXTURE_WIDTH * IMAGE_HEIGHT + (tile % 5) * IMAGE_WIDTH + t;

			// Calculate video offset of floor pixel
			offset = row * RETRO_WIDTH + column;

			// Draw pixel
			int level = litelevel[distance] + ambient_level + flor_lites[xmaze][ymaze];
			if (level > MAXLIGHT) {
				level = MAXLIGHT;
			}
			screen[offset] = litesource[level][image[tileptr]];
		}

		// Step through ceiling pixels
		for (int row = top - 1; row >= VIEWPORT_TOP; --row) {
			// Get ratio of viewer's height to pixel height
			float ratio = (float) (WALL_HEIGHT - viewer_height) / (VIEWPORT_CENTER - row);

			// Get distance to visible pixel
			distance = ratio * VIEWER_DISTANCE / cos(column_angle);

			// Rotate distance to ray angle
			int x = - distance * (sin(radians));
			int y = distance * (cos(radians));

			// Translate relative to viewer coordinates
			x += xview;
			y += yview;

			// Get maze square intersected by ray
			int xmaze = x / FINE_GRIDSIZE;
			int ymaze = y / FINE_GRIDSIZE;

			// Find relevant column of texture map
			int t = (y % FINE_GRIDSIZE) * TEXTURE_WIDTH + (x % FINE_GRIDSIZE);

			// Which graphics tile are we using?
			int tile = ceiling[xmaze][ymaze];

			// Find offset of tile and column in bitmap
			unsigned int tileptr = (tile / 5) * TEXTURE_WIDTH * IMAGE_HEIGHT + (tile % 5) * IMAGE_WIDTH + t;

			// Calculate video offset of ceiling pixel
			offset = row * RETRO_WIDTH + column;

			// Draw pixel
			int level = litelevel[distance] + ambient_level + ceiling_lites[xmaze][ymaze];
			if (level > MAXLIGHT) {
				level = MAXLIGHT;
			}
			screen[offset] = litesource[level][image[tileptr]];
		}
	}
}

void DEMO_Render(double deltatime)
{
	static int ambient_level = 0;
	static double viewing_angle = 0;
	static int viewer_height = 32;
	static double xview = 8 * FINE_GRIDSIZE;
	static double yview = 8 * FINE_GRIDSIZE;

	// Calculate speed
	double walkspeed = deltatime * 200;
	double turnspeed = deltatime * 2;

	const Uint8 * keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_GetScancodeFromKey(SDLK_UP)]) {
		xview -= sin(viewing_angle) * walkspeed;
		yview += cos(viewing_angle) * walkspeed;
	}
	if (keys[SDL_GetScancodeFromKey(SDLK_DOWN)]) {
		xview += sin(viewing_angle) * walkspeed;
		yview -= cos(viewing_angle) * walkspeed;
	}
	if (keys[SDL_GetScancodeFromKey(SDLK_RIGHT)]) {
		viewing_angle += turnspeed;
	}
	if (keys[SDL_GetScancodeFromKey(SDLK_LEFT)]) {
		viewing_angle -= turnspeed;
	}

	unsigned char *buffer = RETRO_GetSurfaceBuffer();
	unsigned char *image = RETRO_GetTextureImage();

	draw_maze(buffer, xview, yview, viewing_angle, viewer_height, image, walls, flor, ceiling, flor_lites, ceiling_lites, ambient_level, litetable, litelevel);
}

void DEMO_Initialize()
{
	RETRO_LoadTexture("assets/raycast.pcx");
	RETRO_SetPaletteFromTexture();
	InitLightTables(RETRO_GetTexturePalette());
}
