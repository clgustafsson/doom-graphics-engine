#define GL_SILENCE_DEPRECATION

#ifdef __APPLE__

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#elif __linux__

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#endif

#include <stdlib.h>
#include <math.h>

#define WALL_COUNT 4
#define SECTOR_COUNT 1
#define COLOR_COUNT 4

const int SCREEN_SCALE = 1;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int SCALED_SCREEN_WIDTH = SCREEN_WIDTH/SCREEN_SCALE;
const int SCALED_SCREEN_HEIGHT = SCREEN_HEIGHT/SCREEN_SCALE;


const int AMOUNT_OF_PIXELS = SCREEN_WIDTH * SCREEN_HEIGHT * 3;
unsigned char *pixelbuffer;

typedef int color;

enum Color {
    NONE,
    RED,
    GREEN,
    BLUE
};

typedef struct vec2 {
    int x, y;
} vec2;

typedef struct vec3 {
    int x, y, z;
} vec3;

typedef struct fvec3 {
  float x, y, z;
} fvec3;

typedef struct wall {
    vec2 p1;
    vec2 p2;
    int texture;
    float distance_from_player;
} wall;

typedef struct sector {
    int first_wall_index;
    int last_wall_index;
    int floor_height;
    int height;
    float distance_from_player;
} sector;

wall walls[WALL_COUNT];

sector sectors[SECTOR_COUNT];

typedef struct player {
    vec3 pos;
    int direction;
} player;

player main_player = (player){(vec3){0, -100, 25}, 0};
fvec3 main_player_movement_between_frames = (fvec3){0, 0, 0};
int main_player_direction_between_frames = 0;


typedef struct keys {
    char w;
    char a;
    char s;
    char d;
    char o;
    char p;
} keys;

keys pressed_keys = {0, 0, 0, 0, 0, 0};

float SIN[360];
float COS[360];

void init_math() {
    float radians_to_degrees = 3.14159265359/180;
    for (int i = 0; i < 360; i++) {
        SIN[i] = sin(radians_to_degrees*i);
        COS[i] = cos(radians_to_degrees*i);
    }
}

float newton_sqrt(float n) {
    float x = n;
    const float accuracy = 0.00001;
    int max_iterations = 0;
    while (((x * x - n) > accuracy || (n - x * x) > accuracy) && max_iterations<25) {
        x = (x + n / x) / 2;
        max_iterations++;
    }
    return x;
}

float distance(vec2 pos1, vec2 pos2) {
    return newton_sqrt((pos1.x-pos2.x)*(pos1.x-pos2.x)+(pos1.y-pos2.y)*(pos1.y-pos2.y));
}

int max(int n1, int n2) {
    if (n1 > n2) {
        return n1;
    }
    return n2;
}

int min(int n1, int n2) {
    if (n1 < n2) {
        return n1;
    }
    return n2;
}

void draw_pixel_on_buffer(vec2 pos, color c);

//adjust these to change the speed of movement and turning
#define UPDATES_PER_FRAME 10
#define NUMBER_OF_SUBDEGREES 5
int SUBDEGREES = 0;
#define MOVEMENT_SCALE 0.5

void move_player() {
    if (pressed_keys.w) {
        main_player_movement_between_frames.x += MOVEMENT_SCALE*SIN[main_player.direction];
        main_player_movement_between_frames.y += MOVEMENT_SCALE*COS[main_player.direction];
    }
    if (pressed_keys.s) {
        main_player_movement_between_frames.x -= MOVEMENT_SCALE*SIN[main_player.direction];
        main_player_movement_between_frames.y -= MOVEMENT_SCALE*COS[main_player.direction];
    }
    if (pressed_keys.a) {
        main_player_movement_between_frames.x -= MOVEMENT_SCALE*COS[main_player.direction];
        main_player_movement_between_frames.y += MOVEMENT_SCALE*SIN[main_player.direction];
    }
    if (pressed_keys.d) {
        main_player_movement_between_frames.x += MOVEMENT_SCALE*COS[main_player.direction];
        main_player_movement_between_frames.y -= MOVEMENT_SCALE*SIN[main_player.direction];
    }

    if (pressed_keys.o) {

        SUBDEGREES--;

        if (SUBDEGREES == -NUMBER_OF_SUBDEGREES) {
          SUBDEGREES = 0;
          main_player_direction_between_frames -= 1;
          if (main_player_direction_between_frames == -1) {
            main_player_direction_between_frames = 359;
          }
        }
    }
    if (pressed_keys.p) {

        SUBDEGREES++;

        if (SUBDEGREES == NUMBER_OF_SUBDEGREES) {
          SUBDEGREES = 0;
          main_player_direction_between_frames += 1;
          if (main_player_direction_between_frames == 360) {
            main_player_direction_between_frames = 0;
          }
        }

    }

}

void update_main_player_movement() {
  main_player.pos.x += main_player_movement_between_frames.x;
  main_player.pos.y += main_player_movement_between_frames.y;
  main_player.pos.z += main_player_movement_between_frames.z;
  main_player_movement_between_frames = (fvec3){0, 0, 0};
}

void update_main_player_direction() {
  main_player.direction += main_player_direction_between_frames;
  if (main_player.direction > 359) {
    main_player.direction -= 360;
  }
  if (main_player.direction < 0) {
    main_player.direction += 360;
  }
  main_player_direction_between_frames = 0;
}

void clear_pixelbuffer() {
    for (int i = 0; i < AMOUNT_OF_PIXELS; i++) {
        pixelbuffer[i] =  0;
    }
}

void draw_pixel_on_buffer(vec2 pos, color c) {
    color rgb[3];
    if (c == RED) {
        rgb[0] = 255; rgb[1] = 0; rgb[2] = 0;
    }
    else if (c == GREEN) {
        rgb[0] = 0; rgb[1] = 255; rgb[2] = 0;
    }
    else if (c == BLUE) {
        rgb[0] = 0; rgb[1] = 0; rgb[2] = 255;
    }
    pos = (vec2){pos.x*SCREEN_SCALE, pos.y*SCREEN_SCALE};
    for (int w=0; w < SCREEN_SCALE; w++) {
        for (int h=0; h < SCREEN_SCALE; h++) {
            pixelbuffer[((pos.y+h)*SCREEN_WIDTH+(pos.x+w))*3] = rgb[0];
            pixelbuffer[((pos.y+h)*SCREEN_WIDTH+(pos.x+w))*3 + 1] = rgb[1];
            pixelbuffer[((pos.y+h)*SCREEN_WIDTH+(pos.x+w))*3 + 2] = rgb[2];
        }
    }
}



void end_line_behind_player(vec3 *pos1, vec3 *pos2) {
    float d1 = pos1->y;
    float d2 = pos2->y;
    float d = d1-d2;
    if (d == 0) {
        d = 1;
    }
    float s = d1/d;
    pos1->x += s*(pos2->x-pos1->x);
    pos1->y += s*(pos2->y-pos1->y);
    pos1->z += s*(pos2->z-pos1->z);

    if (pos1->y == 0) {
        pos1->y = 1;
    }
}

void render_wall(vec2 pos1, vec2 pos2, int pos1_top, int pos2_top, color color) {

    if (pos2.x < pos1.x) {
        int temp = pos2.x;
        pos2.x = pos1.x;
        pos1.x = temp;
        temp = pos2.y;
        pos2.y = pos1.y;
        pos1.y = temp;
        temp = pos2_top;
        pos2_top= pos1_top;
        pos1_top = temp;
    }




    int dx = pos2.x - pos1.x;
    int dy = pos2.y - pos1.y;

    int dy_top = pos2_top - pos1_top;


    if (dx == 0) {
        dx = 1;
    }
    int old_pos1x = pos1.x;
    if (pos1.x < 1) {
        pos1.x = 1;
    }
    if (pos2.x > SCALED_SCREEN_WIDTH-1) {
        pos2.x = SCALED_SCREEN_WIDTH-1;
    }

    for (int x = pos1.x; x <= pos2.x; x++) {
        int bottom_y = dy*(x-old_pos1x+0.5)/dx+pos1.y;
        int top_y = dy_top*(x-old_pos1x+0.5)/dx+pos1_top;


        if (bottom_y < min(pos1.y, pos2.y)) {
            bottom_y = min(pos1.y, pos2.y);
        }
        if (top_y > max(pos1_top, pos2_top)) {
            top_y = max(pos1_top, pos2_top);
        }

        if (bottom_y < 1) {
            bottom_y = 0;
        }
        if (bottom_y > SCALED_SCREEN_HEIGHT) {
            continue;
        }
        if (top_y < 1) {
            continue;
        }
        if (top_y > SCALED_SCREEN_HEIGHT) {
            top_y = SCALED_SCREEN_HEIGHT;
        }

        if (color > COLOR_COUNT) {

        }
        else {
            for (int y = bottom_y; y <= top_y; y++) {
                draw_pixel_on_buffer((vec2){x, y}, color);
            }
        }

    }

}


void render() {

    int i = 1;
    while (i < SECTOR_COUNT) {
        int j = i;
        while (j > 0 && sectors[j-1].distance_from_player < sectors[j].distance_from_player) {
            sector temp = sectors[j];
            sectors[j] = sectors[j-1];
            sectors[j-1] = temp;
            j--;
        }
        i++;
    }


    for (int sector_i = 0; sector_i < SECTOR_COUNT; sector_i++) {
        sector sector = sectors[sector_i];

        int walls_in_sector = 1+sector.last_wall_index-sector.first_wall_index;

        if (walls_in_sector > 1) {
            int i = sector.first_wall_index+1;
            while (i <= sector.last_wall_index) {
                int j = i;
                while (j > 0 && walls[j-1].distance_from_player < walls[j].distance_from_player) {
                    wall temp = walls[j];
                    walls[j] = walls[j-1];
                    walls[j-1] = temp;
                    j--;
                }
                i++;
            }
        }



        for (int wall_i = sector.first_wall_index; wall_i <= sector.last_wall_index; wall_i++) {

            wall wall = walls[wall_i];



            vec2 pos1 = wall.p1;
            vec2 pos2 = wall.p2;

            pos1.x -= main_player.pos.x;
            pos1.y -= main_player.pos.y;
            pos2.x -= main_player.pos.x;
            pos2.y -= main_player.pos.y;

            vec3 point1;
            vec3 point2;

            point1.x = pos1.x*COS[main_player.direction]-pos1.y*SIN[main_player.direction];
            point2.x = pos2.x*COS[main_player.direction]-pos2.y*SIN[main_player.direction];

            point1.y = pos1.y*COS[main_player.direction]+pos1.x*SIN[main_player.direction];
            point2.y = pos2.y*COS[main_player.direction]+pos2.x*SIN[main_player.direction];

            point1.z = sector.floor_height-main_player.pos.z;
            point2.z = sector.floor_height-main_player.pos.z;

            vec3 point3 = point1;
            vec3 point4 = point2;
            point3.z += sector.height;
            point4.z += sector.height;

            float distance_to_center_of_wall = distance((vec2){0,0}, (vec2){0,(point1.y+point2.y)/2});

            walls[wall_i].distance_from_player = (point1.y+point2.y)/2.0;
            sectors[sector_i].distance_from_player += distance_to_center_of_wall;


            if (point1.y < 1 && point2.y < 1) {
                continue;
            }
            else if (point1.y < 1) {
                end_line_behind_player(&point1, &point2);
                end_line_behind_player(&point3, &point4);

            }
            else if (point2.y < 1) {
                end_line_behind_player(&point2, &point1);
                end_line_behind_player(&point4, &point3);
            }


            vec2 point1_screen_pos = (vec2){point1.x*200/point1.y+(SCALED_SCREEN_WIDTH/2), point1.z*200/point1.y+(SCALED_SCREEN_WIDTH/2)};
            vec2 point2_screen_pos = (vec2){point2.x*200/point2.y+(SCALED_SCREEN_WIDTH/2), point2.z*200/point2.y+(SCALED_SCREEN_WIDTH/2)};
            vec2 point3_screen_pos = (vec2){point3.x*200/point1.y+(SCALED_SCREEN_WIDTH/2), point3.z*200/point1.y+(SCALED_SCREEN_WIDTH/2)};
            vec2 point4_screen_pos = (vec2){point4.x*200/point2.y+(SCALED_SCREEN_WIDTH/2), point4.z*200/point2.y+(SCALED_SCREEN_WIDTH/2)};


            render_wall(point1_screen_pos, point2_screen_pos, point3_screen_pos.y, point4_screen_pos.y, wall.texture);

        }
        sectors[sector_i].distance_from_player /= walls_in_sector;

    }



}



void display() {
    clear_pixelbuffer();

    update_main_player_movement();
    update_main_player_direction();
    render();
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixelbuffer);
    glutSwapBuffers();
}


void idle() {
    for (int i = 0; i < UPDATES_PER_FRAME; ++i) {
        move_player();
    }
    glutPostRedisplay();
}


void keys_down(unsigned char key,int x,int y) {
    if(key=='w'==1){ pressed_keys.w = 1; }
    if(key=='a'==1){ pressed_keys.a = 1; }
    if(key=='s'==1){ pressed_keys.s = 1; }
    if(key=='d'==1){ pressed_keys.d = 1; }
    if(key=='o'==1){ pressed_keys.o = 1; }
    if(key=='p'==1){ pressed_keys.p = 1; }
}

void keys_up(unsigned char key,int x,int y) {
    if(key=='w'==1){ pressed_keys.w = 0; }
    if(key=='a'==1){ pressed_keys.a = 0; }
    if(key=='s'==1){ pressed_keys.s = 0; }
    if(key=='d'==1){ pressed_keys.d = 0; }
    if(key=='o'==1){ pressed_keys.o = 0; }
    if(key=='p'==1){ pressed_keys.p = 0; }
}

void init_level() {
    sectors[0] = (sector){0, 3, 0, 40, 0};
    walls[0] = (wall){(vec2){-50, 0}, (vec2){50, 0}, RED, 0};
    walls[1] = (wall){(vec2){-50, 0}, (vec2){-50, -50}, GREEN, 0};
    walls[2] = (wall){(vec2){50, -50}, (vec2){50, 0}, BLUE, 0};
}

void init() {
    init_math();
    init_level();
}

int main(int argc, char** argv) {
    init();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("");

    pixelbuffer = (unsigned char*) malloc(AMOUNT_OF_PIXELS);

    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glutKeyboardFunc(keys_down);
    glutKeyboardUpFunc(keys_up);

    glutMainLoop();

    free(pixelbuffer);
    return 0;
}
