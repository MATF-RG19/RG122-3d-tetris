#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>

#define TIMER_INTERVAL 20
#define TIMER_ID 1

/*Definisemo granice po x,y i z kordinati*/
#define X1 -4
#define X2  4
#define Y1 -4
#define Y2  4
#define Z1  0
#define Z2  8

//Za translaciju koja se vrsi pomocu strelica na tastaturi
int Px=0,Py=0;
int Pz=10;

/*Definisemo granicu matrice*/
#define XY 8
#define Z 15

/*Koordinate misa*/
int mouse_x = 0;
int mouse_y = 0;

static float matrix[16];

//Dimenzije prozora 
static int window_width, window_height;
/*Granice i azuriranja istih*/
int granice=0;

//Racuna koliko je figura palo
int palo=0;

/*Niz koji se sastoji od random brojeva*/
#define MAX 100
int array[MAX];
int rand_count=0;
//Callback funkcije
static void on_display(void);
static void on_reshape(int width, int height);
static void on_keyboard(unsigned char key, int x, int y);
static void on_mouse(int  button, int state, int x, int y);
static void on_timer(int id);
static void on_arrow(int key, int x, int y);
static void on_motion(int x, int y);


static void set_material(int id);
void zaustavljanjeFigure(void);
void rotiraj(void);

void crtanjeDelovaScene(void);
void choose_fig(int id);

/*Ove funkcije nam govore koliko figuru mozemo poremiti u jednom od 4 smera*/
void graniceFigure(int id);
void izmeni_granice(int id);
void izmeni1(void);
void izmeni2(void);
void izmeni3(void);
void izmeni4(void);
void izmeni5(void);


int ***status;
int ***alloc_mat(int zlen, int ylen, int xlen);
void free_mat(int ***status, int zlen, int ylen);

/*Popunjavamo matircu stanja padajucim figurama*/
void update_status(int id);

void sudar(int id);

int animation_ongoing;
int parametar;

/*Definisemo strukturu u kojoj cemo cuvati stanja rotacije */
struct rot_stanje {
    bool da_ne;    
    float x,y,z;      
    int trenutna; /* trenutna osa rotacije 1 je za x, 2 za y, 3 i z */
} rotacija;
int r_count=0;

struct granice_figura{
    int levo;
    int desno;
    int gore;
    int dole;
}granica;

/*Najniza */
struct lowest{
    int x,y,z;
}low[4];

int main(int argc, char **argv)
{
    //Inicijalizacija GLUTa
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    
    //Kreira se prozor
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(1000, 1000);
    glutCreateWindow(argv[0]);

    mouse_x = 0;
    mouse_y = 0;

    //Registruju se funkcije za obradu dogadjaja
    glutDisplayFunc(on_display);
	glutReshapeFunc(on_reshape);
    glutKeyboardFunc(on_keyboard);
    glutSpecialFunc(on_arrow);
    glutMouseFunc(on_mouse);
    glutMotionFunc(on_motion);
    
    /*Inicijalizujemo stanje rotacije*/
    rotacija.da_ne = false;
    rotacija.x = rotacija.y = rotacija.z = 0;
    rotacija.trenutna = 0;
    
    /*Inicijalizujemo sve najnize tacke figure,kojih ukupno moze da ima 4, na 0*/
    int i;
    for(i=0;i<4;i++){
        low[i].x=0;
        low[i].y=0;
        low[i].z=0;
    }
    
    /*Inicijalizujemo niz random brojeva koji cemo koristiti za crtanja figure*/

   
    for(rand_count=0; rand_count<MAX; rand_count++){
        array[rand_count]=rand()%6;
    }
        
    /*Alociramo memoriju da bismo pamtili spustene figure*/
    status=alloc_mat(Z,XY,XY);
    if(status == NULL)
        exit(EXIT_FAILURE);
    
    for(int i = 0; i < Z; i++)
        for(int j = 0; j < XY; j++)
            for(int k = 0; k < XY; k++)
                status[i][j][k]=0;
    

    /* Postavlja se pozadina */
    glClearColor(0.25, 0.25, 0.25, 0);
	glEnable(GL_DEPTH_TEST);
    
    //Normalizacija
    glEnable(GL_NORMALIZE);

    //Inicijalizacija matrice rotacije
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	
    //Glavna petlja
    glutMainLoop();

    return 0;
}


static void on_reshape(int width, int height)
{
    //Pamte se sirina i visina prozora
    window_width = width;
    window_height = height;
    //Postavlja se viewport
    glViewport(0, 0, width, height);
    //Postavljaju se parametri projekcije
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float) width / height, 1, 1800);
}

static void on_display(void)
{
    //Postavlja se boja piksela
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Postavlja se tacka pogleda
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 18, 
              0, 0, 0, 
              0, 1, 0);
    
    //Normalizacija vektora 
    glNormal3f(0,0,1);
    
    /*Da vrati brojac na 0*/
      if(rand_count == MAX)
        rand_count=0;
    //Primenjuje se matrica rotacije
    glMultMatrixf(matrix);
    
    //Podesavamo osvetljenje
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_position[] = { 0, -1, 1, 0};
	GLfloat light_ambient[] = { 0.1, 0.2, 0.3, 1 };
	GLfloat light_diffuse[] = { 1, 1, 1, 1 };
    GLfloat light_specular[] = { 1, 1, 1, 1 };
    
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    //Postavljamo materjale
    set_material(13);
    // Crtanje delova scene 
    crtanjeDelovaScene();
    // Zaustavljanje figure 
    zaustavljanjeFigure();
    //Salje se slika na ekran 
    glutSwapBuffers();
}


static void on_timer(int id)
{
    //Povecavamo proteklo vreme
    parametar +=2;
     glutPostRedisplay();
    
    if(animation_ongoing){
        if(parametar % 50 == 0)
            Pz = Pz-1;
        glutTimerFunc(TIMER_INTERVAL,on_timer,TIMER_ID);
    }
}

static void on_keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 27:
        printf("\n------------- KRAJ IGRE -------------\n\n****Palo je %d figura!****\n\n",palo);
        exit(0);
        break;
    case 'g':
        if(!animation_ongoing){
            animation_ongoing = 1;
            glutTimerFunc(TIMER_INTERVAL,on_timer, TIMER_ID);
        }
        break;
    case 'p':
    case 'P':
        /* Zaustavlja se animacija. */
        animation_ongoing = 0;
        break;        
    case 'j':
    case 'J':
        /* Rotacija po x osi */
        if(animation_ongoing && (r_count==0 || rotacija.trenutna ==1)){
            rotacija.trenutna = 1;
            rotacija.da_ne = true;
            rotiraj();
        }
        break;

    case 'k':
    case 'K':
        /* Rotacija po y osi */ 
        if(animation_ongoing && (r_count==0 || rotacija.trenutna ==2)){
            rotacija.trenutna = 2;
            rotacija.da_ne = true;
            rotiraj();
        }
        break;
    
    case 'l':
    case 'L':
        /* Rotacija po z osi */
        if(animation_ongoing && (r_count==0 || rotacija.trenutna ==3)){
            rotacija.trenutna = 3;
            rotacija.da_ne= true;
            rotiraj();
        }
        break;
  }
}

static void on_arrow(int key, int x, int y)
{
    /*Ovde vec i sami tasteri opisuju kretanje figure*/
    switch (key) {
        case GLUT_KEY_RIGHT:
            if(granica.desno > Px && animation_ongoing)
                Px++;
            break;
        case GLUT_KEY_LEFT:
            if(-granica.levo < Px && animation_ongoing)
                Px--;
            break;
        case GLUT_KEY_UP:
            if(granica.gore > Py && animation_ongoing)
                Py++;
            break;
        case GLUT_KEY_DOWN:
            if(-granica.dole < Py && animation_ongoing)
                Py--;
            break;
        
  }
  glutPostRedisplay();
}

static void on_mouse(int buttun, int state, int x, int y)
{
    /* Pamti se pozicija misa. */
    mouse_x = x;
    mouse_y = y;
}

static void on_motion(int x, int y)
{
    /* Promene pozicije misa. */
    int delta_x, delta_y;

    delta_x = x - mouse_x;
    delta_y = y - mouse_y;

    /* Pamti se nova pozicija misa. */
    mouse_x = x;
    mouse_y = y;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        glLoadIdentity();
        glRotatef(45 * (float) delta_x / window_width, 0, 1, 0);
        glRotatef(45 * (float) delta_y / window_height, 1, 0, 0);
        glMultMatrixf(matrix);

        glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glPopMatrix();

    glutPostRedisplay();

}

void crtanjeDelovaScene(void)
{   
    /*Crtamo postolje */
    int u,v,c;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int k=Z1;
    for(int j=Y1; j<Y2; j++){
    	glBegin(GL_QUAD_STRIP);
       	 	glColor3f(0.9, 0.9, 0.9);
		int i=X1;
		glVertex3f(i , -j, k);
        glVertex3f(i , -(j+1), k);
		for(i=X1+1; i<=X2; i++){
        		glVertex3f(i , - j , k);
        		glVertex3f(i , -(j+1) , k);
        }

    	glEnd();
    }

    /*Bocne strane*/

    v=Y1;
    for(u=X1; u < X2; u++){
      glBegin(GL_QUAD_STRIP);
        glColor3f(0.9, 0.2, 0.7);
        for(c=Z1; c <= Z2; c++){
            glVertex3f(u , -v, c);
            glVertex3f((u+1) , -v, c);
        }
    glEnd();
    }
    
     u=X1;
    for(v=Y1; v < Y2; v++){
      glBegin(GL_QUAD_STRIP);
        glColor3f(0.8, 0.8, 0.1);
        for(c=Z1; c <= Z2; c++){
            glVertex3f(-u , -v , c);
            glVertex3f(-u , -(v+1) , c);
        }
    glEnd();
    }
    for(v=Y1; v < Y2; v++){
      glBegin(GL_QUAD_STRIP);
        glColor3f(0.2, 0.6, 0.3);
        for(c=Z1; c <= Z2; c++){
            glVertex3f(u , -v , c);
            glVertex3f(u , -(v+1) , c);
        }
    glEnd();
    }


    /*Crtamo matricu stanja*/

    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	for(int i = 0; i < Z; i++){
        set_material(i);
        for(int j = 0; j < XY; j++)
            for(int k = 0; k < XY; k++)
                if(status[i][j][k]==1){
                    glPushMatrix();
                        glTranslatef(0.5,0.5,0.5);
                        glTranslatef(k-4,j-4,i);
                        glutSolidCube(1);
                    glPopMatrix();
                }     
    }
    
    /*Crtamo figuru*/
    glPushMatrix();
        set_material(array[rand_count]);
        glTranslatef(0.5,0.5,0.5);
        glTranslatef(Px,Py,Pz);
        glRotatef(rotacija.x, 1, 0, 0);
        glRotatef(rotacija.y, 0, 1, 0);
        glRotatef(rotacija.z, 0, 0, 1);
        
        choose_fig(array[rand_count]);
    glPopMatrix();
    
}

void zaustavljanjeFigure(void)
{

for(int c = Z-1; c >= 0; c--){
    for(int v = 0; v < XY; v++){
        for(int u = 0; u < XY; u++){
            for(int i=0;i<4;i++){
                if((status[c][v][u]==1 && c==(Pz+low[i].z-1) && v==(low[i].y+Py) && u==(low[i].x+Px)) || Pz <= 0 || Pz+low[i].z <=0){
                    animation_ongoing = 0;
                    parametar = 0;              
                    update_status(array[rand_count]);
                    palo++;
                    /*Vracamo promenljive na pocetne vrednosti*/
                    Px=0;
                    Py=0;
                    Pz=10;
                    rotacija.x=0;
                    rotacija.y=0;
                    rotacija.z=0;  
                    int i;
                    for(i=0;i<4;i++){
                        low[i].x=0;
                        low[i].y=0;
                        low[i].z=0;
                    }
                    granice=0;   

        
                    /*Proveravamo da li je kraj igre*/
                    if(status[Pz][4][4]==1){
                        printf("******** GAME OVER ********\n****Palo je %d figura!****\n\n",palo);
                        free_mat(status, Z, XY);
                        exit(0);
                    }                   
                    rand_count++;
                    animation_ongoing=1;
                }
            }
        }
    }
}

}


static void set_material(int id)
{
    GLfloat ambient_coeffs[] = { 0.3, 0.3, 0.3, 1 };

    GLfloat diffuse_coeffs[] = { 1, 1, 1, 1 };

    GLfloat specular_coeffs[] = { 0.8, 0.8, 0.8, 1 };

    GLfloat shininess = 60;
    
    switch (id) {
        case 0:
			diffuse_coeffs[0] = 0;
            diffuse_coeffs[1] = 0.1;
            diffuse_coeffs[2] = 0.75;

            ambient_coeffs[0] = 0.3;
            ambient_coeffs[1] = 0.2;
            ambient_coeffs[2] = 0.0;
            break;
        case 1:
            diffuse_coeffs[0] = 0.14;
            diffuse_coeffs[1] = 0.95;
            diffuse_coeffs[2] = 0.8470;

            ambient_coeffs[0] = 0;
            ambient_coeffs[1] = 0.3;
            ambient_coeffs[2] = 0.3;
            break;
        case 2:

            diffuse_coeffs[0] = 0.7;
            diffuse_coeffs[1] = 0.19;
            diffuse_coeffs[2] = 0.7;

            ambient_coeffs[0] = 0.3;
            ambient_coeffs[1] = 0.0;
            ambient_coeffs[2] = 0.1;
            break;
        case 3:

            diffuse_coeffs[0] = 1;
            diffuse_coeffs[1] = 0.19;
            diffuse_coeffs[2] = 0.1;
           
            ambient_coeffs[0] = 0.8;
            ambient_coeffs[1] = 0.1;
            ambient_coeffs[2] = 0.1;
            break;
        case 4:

            diffuse_coeffs[0] = 1;
            diffuse_coeffs[1] = 0.796;
            diffuse_coeffs[2] = 0.2313;

            ambient_coeffs[0] = 0.5;
            ambient_coeffs[1] = 0.3;
            ambient_coeffs[2] = 0.0;
            break;
        case 5:
            diffuse_coeffs[0] = 0.75;
            diffuse_coeffs[1] = 0.0;
            diffuse_coeffs[2] = 0.0;

            ambient_coeffs[0] = 0.5;
            ambient_coeffs[1] = 0.1;
            ambient_coeffs[2] = 0.0;
            break;
       
        
    }
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_coeffs);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_coeffs);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular_coeffs);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

}


void choose_fig(int id)
{    
    /*Nasumicno biramo figuru koju cemo iscrtati*/
    switch(id){
        case 0:
            glColor3f(0.2,.1,.9);
    			glPushMatrix();
        			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        			glutSolidCube(1);
    			glPopMatrix();

    		if(granice == 0){
        		graniceFigure(0);   
    }
            break;
        case 1:
            glColor3f(0.9,0.9,0.0);
    		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    		glPushMatrix();
        		glPushMatrix();
            		glutSolidCube(1);
        		glPopMatrix();
        		glPushMatrix();
            		glTranslatef(1,0,0);
            		glutSolidCube(1);
        		glPopMatrix();
        		glPushMatrix();
            		glTranslatef(-1,0,0);
            		glutSolidCube(1);
        		glPopMatrix();
    
        		glTranslatef(-1,1,0);
        		glutSolidCube(1);
    
    		glPopMatrix();
    
    		if(granice == 0){
        		graniceFigure(1);   
    		}
            break;
			
        case 2:
            glColor3f(0.7,.1,.3);
   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    		glPushMatrix();
        		glutSolidCube(1);
    			glTranslatef(0,1,0);
        		glutSolidCube(1);
        		glTranslatef(0,-1,0);
    		    glTranslatef(0,-1,0);
        		glutSolidCube(1);
        		glTranslatef(0,1,0);
    			glTranslatef(1,0,0);
        		glutSolidCube(1);
    		glPopMatrix();
    
    		if(granice == 0){
        		graniceFigure(2);   
    		}
            break;
        case 3:
			glColor3f(0.0,.8,.9);
    		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   
    		glPushMatrix();
        		glutSolidCube(1);
    			glTranslatef(-1,0,0);
        		glutSolidCube(1);
        		glTranslatef(1,0,0);
        		glTranslatef(0,-1,0);
        		glutSolidCube(1);
        		glTranslatef(0,1,0);    
        		glTranslatef(1,-1,0);
        		glutSolidCube(1);
    		glPopMatrix();
    
    		if(granice == 0)
        		graniceFigure(3);   
    		           
	 	break;

        case 4:
            glColor3f(0.5,.9,.2);
    		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    		glPushMatrix();
                glutSolidCube(1);
                glTranslatef(1,0,0);
                glutSolidCube(1);
                glTranslatef(-1,0,0);
                glTranslatef(0,-1,0);
                glutSolidCube(1);
                glTranslatef(0,1,0);
                glTranslatef(1,-1,0);
                glutSolidCube(1);
            glPopMatrix();
    
            if(granice == 0)
                graniceFigure(4);   
            break;
        case 5:            
            glColor3f(1.0,0.6,0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
            glPushMatrix();
                glutSolidCube(1);
                glTranslatef(0,1,0);
                glutSolidCube(1);
                glTranslatef(0,-1,0);
                glTranslatef(0,2,0);
                glutSolidCube(1);
                glTranslatef(0,-2,0);   
                glTranslatef(0,-1,0);
                glutSolidCube(1);
            glPopMatrix();
    
            if(granice == 0)
        graniceFigure(5);   
            break;
    }
}

void rotiraj(void)
{
/*Rotiramo za ugao od 90 stepeni*/
switch (rotacija.trenutna){
    case 1:/*Po X osi*/
        rotacija.x += 90;
        rotacija.da_ne = false;
        r_count++;
        izmeni_granice(array[rand_count]);
        break;
    case 2:/*Po Y osi*/
        rotacija.y += 90;
        rotacija.da_ne = false;
        r_count++;
        izmeni_granice(array[rand_count]);
        break;
    case 3:/*Po Z osi*/
        rotacija.z += 90;
        rotacija.da_ne = false;
        r_count++;
        izmeni_granice(array[rand_count]);
        break;
}

if(r_count == 4)
    r_count=0;

/*Na kraju treba da proverimo da li je figura ostala u okviru postolja, tj. mreze i ako nije,
treba je vratiti*/
if(granica.gore < Py)
    Py-=Py-granica.gore;
else if(-granica.dole > Py)
    Py+=-(Py-granica.dole);
else if(granica.desno < Px)
    Px-=Px-granica.desno;
else if(-granica.levo > Px)
    Px+=-(Px-granica.levo);

glutPostRedisplay();
}

void graniceFigure(int id)
{
/*Koliko se svaka figura moze maksimalno kretati po polju*/
    switch(id){
        case 0:    
            granica.levo=4;
            granica.desno=3;
            granica.gore=3;
            granica.dole=4;
            granice++;

            sudar(id);
            break;
        case 1:
            granica.levo=3;
            granica.desno=2;
            granica.gore=2;
            granica.dole=4;
            granice++;
            sudar(id);
            break;
        case 2:
            granica.levo=4;
            granica.desno=2;
            granica.gore=2;
            granica.dole=3;
            granice++;
            sudar(id);
            break;
        case 3:
            granica.levo=3;
            granica.desno=2;
            granica.gore=3;
            granica.dole=3;
            granice++;
            sudar(id);
            break;
        case 4:
            granica.levo=4;
            granica.desno=2;
            granica.gore=3;
            granica.dole=3;
            granice++;
            sudar(id);
            break;
        case 5:
            granica.levo=4;
            granica.desno=3;
            granica.gore=1;
            granica.dole=3;
            granice++;
            sudar(id);
            break;
    }
}

void sudar(int id)
{
/*Postavljamo maksimalno pomenje figura i definicemo tacke za detekciju sudara*/
    switch(id){
        case 0:    
            low[0].x=4;
            low[0].y=4;
            break;
        case 1:
            low[0].x=4;
            low[0].y=4;
            low[1].x=3;
            low[1].y=4;
            low[2].x=3;
            low[2].y=3;
            low[3].x=5;
            low[3].y=4;
            break;
        case 2:
          low[0].x=4;
            low[0].y=4;
            low[1].x=3;
            low[1].y=4;
            low[2].x=4;
            low[2].y=3;
            low[3].x=5;
            low[3].y=4;

            break;
        case 3:
            low[0].x=4;
            low[0].y=4;
            low[1].x=3;
            low[1].y=4;
            low[2].x=4;
            low[2].y=5;
            low[3].x=5;
            low[3].y=5;
            break;
        case 4:
             low[0].x=4;
            low[0].y=4;
            low[1].x=3;
            low[1].y=4;
            low[2].x=4;
            low[2].y=5;
            low[3].x=5;
            low[3].y=5;
            break;
        case 5:
            low[0].x=4;
            low[0].y=4;
            low[1].x=4;
            low[1].y=3;
            low[2].x=4;
            low[2].y=2;
            low[3].x=4;
            low[3].y=5;
            break;
    }
}

void izmeni_granice(int id)
{
    switch(id){
        case 1:
            izmeni1();
            break;
        case 2:
            izmeni2();
            break;
        case 3:
            izmeni3();
            break;
        case 4:
            izmeni4();
            break;
        case 5:
            izmeni5();
            break;
    }
}

void izmeni1(void)
{
    switch(rotacija.trenutna){
        case 1: /*Po X osi*/
            if(r_count == 1){
                granica.gore +=1;
            }else if(r_count == 2){
                granica.dole -=1;
            }else if(r_count == 3){
                granica.dole +=1;
            }else if(r_count == 4){
                granica.levo=3;
                granica.desno=2;
                granica.gore=2;
                granica.dole=4;
            }
            break;
        case 2:/*Po Y osi*/
            if(r_count == 1){
                granica.levo +=1;
                granica.desno +=1;
            }else if(r_count == 2){
                granica.levo -=1;
                granica.desno -=1;
            }else if(r_count == 3){
                granica.levo +=1;
                granica.desno +=1;
            }else if(r_count == 4){
                granica.levo=3;
                granica.desno=2;
                granica.gore=2;
                granica.dole=4;
            }
            break;
        case 3:/*Po Z osi*/
            if(r_count == 1){
                granica.dole -=1;
                granica.desno +=1;
            }else if(r_count == 2){
                granica.gore +=1;
                granica.desno -=1;
            }else if(r_count == 3){
                granica.levo +=1;
                granica.gore -=1;
            }else if(r_count == 4){
                granica.levo=3;
                granica.desno=2;
                granica.gore=2;
                granica.dole=4;
            }
            break;
    }
}

void izmeni2(void)
{
switch(rotacija.trenutna){
        case 1:/*Po x osi*/
            if(r_count == 1){
                granica.gore +=1;
                granica.dole +=1;
            }else if(r_count == 2){
                granica.gore -=1;
                granica.dole -=1;
            }else if(r_count == 3){
                granica.gore +=1;
                granica.dole +=1;
            }else if(r_count == 4){
                granica.levo=4;
                granica.desno=2;
                granica.gore=2;
                granica.dole=3;
            }
            break;
        case 2:/*Po y osi*/
            if(r_count == 1){
                granica.desno +=1;
            }else if(r_count == 2){
                granica.levo -=1;
            }else if(r_count == 3){
                granica.levo +=1;
            }else if(r_count == 4){
                granica.levo=4;
                granica.desno=2;
                granica.gore=2;
                granica.dole=3;
            }
            break;
        case 3:/*Po Z osi*/
            if(r_count == 1){
                granica.dole +=1;
                granica.levo -=1;
            }else if(r_count == 2){
                granica.dole -=1;
                granica.desno +=1;
            }else if(r_count == 3){
                granica.desno -=1;
                granica.gore +=1;
            }else if(r_count == 4){
                granica.levo=4;
                granica.desno=2;
                granica.gore=2;
                granica.dole=3;
            }
    
            break;
    }
}
void izmeni3(void)
{
switch(rotacija.trenutna){
        case 1: /*Po X osi*/
            if(r_count == 1){
                granica.dole +=1;
            }else if(r_count == 2){
                granica.gore -=1;
            }else if(r_count == 3){
                granica.gore +=1;
            }else if(r_count == 4){
                granica.levo=3;
                granica.desno=2;
                granica.gore=3;
                granica.dole=3;
            }
            break;
        case 2: /*Po Y osi*/
            if(r_count == 1){
                granica.levo +=1;
                granica.desno +=1;
            }else if(r_count == 2){
                granica.levo -=1;
                granica.desno -=1;
            }else if(r_count == 3){
                granica.levo +=1;
                granica.desno +=1;
            }else if(r_count == 4){
                granica.levo=3;
                granica.desno=2;
                granica.gore=3;
                granica.dole=3;
            }
            break;
        case 3: /*Po Z osi*/
            if(r_count == 1){
                granica.gore -=1;
                granica.levo +=1;
            }else if(r_count == 2){
                granica.dole +=1;
                granica.levo -=1;
            }else if(r_count == 3){
                granica.desno +=1;
                granica.dole -=1;
            }else if(r_count == 4){
                granica.levo=3;
                granica.desno=2;
                granica.gore=3;
                granica.dole=3;
            }
    
            break;
    }    
}


void izmeni4(void)
{
switch(rotacija.trenutna){
    case 1: /*Po X osi*/
        if(r_count == 1){
            granica.dole +=1;
        }else if(r_count == 2){
            granica.gore -=1;
        }else if(r_count == 3){
            granica.gore +=1;
        }else if(r_count == 4){
            granica.levo=4;
            granica.desno=2;
            granica.gore=3;
            granica.dole=3;
        }
        break;
    case 2: /*Po Y osi*/
        if(r_count == 1){
            granica.desno +=1;
        }else if(r_count == 2){
            granica.levo -=1;
        }else if(r_count == 3){
            granica.levo +=1;
        }else if(r_count == 4){
            granica.levo=4;
            granica.desno=2;
            granica.gore=3;
            granica.dole=3;
        }
        break;
    case 3: /*Po Z osi*/
        if(r_count == 1){
            granica.gore -=1;
            granica.dole +=1;
        }else if(r_count == 2){
            granica.desno +=1;
            granica.levo -=1;
        }else if(r_count == 3){
            granica.gore +=1;
            granica.dole -=1;
        }else if(r_count == 4){
            granica.levo=4;
            granica.desno=2;
            granica.gore=3;
            granica.dole=3;
        }
        break;
    }
}

void izmeni5(void)
{
switch(rotacija.trenutna){
    case 1: /*Po X osi*/
        if(r_count == 1){
            granica.gore +=2;
            granica.dole +=1;
        }else if(r_count == 2){
            granica.gore -=1;
            granica.dole -=2;
        }else if(r_count == 3){
            granica.gore +=1;
            granica.dole +=2;
        }else if(r_count == 4){
            granica.levo=4;
            granica.desno=3;
            granica.gore=1;
            granica.dole=3;
        }
        break;
    case 3: /*Po Z osi*/
        if(r_count == 1){
            granica.levo -=2;
            granica.desno -=1;
            granica.gore +=2;
            granica.dole +=1;
        }else if(r_count == 2){
            granica.levo +=2;
            granica.desno +=1;
            granica.gore -=1;
            granica.dole -=2;
        }else if(r_count == 3){
            granica.levo -=1;
            granica.desno -=2;
            granica.gore +=1;
            granica.dole +=2;
        }else if(r_count == 4){
            granica.levo=4;
            granica.desno=3;
            granica.gore=1;
            granica.dole=3;
        }
        break;
    }
}

void update_status(int id)
{
int a=4+Px;
int b=4+Py;
int c=Pz;
status[c][b][a]=1;

switch(id){
        case 1:
            if(r_count == 0){
                status[Pz][b][a+1]=1;
                status[Pz][b][a-1]=1;
                status[Pz][b+1][a-1]=1;
            }
            break;
        case 2:
            if(r_count == 0){
                status[Pz][b+1][a]=1;
                status[Pz][b][a+1]=1;
                status[Pz][b-1][a]=1;
            }
            break;
        case 3:
            if(r_count == 0){
                status[Pz][b][a-1]=1;
                status[Pz][b-1][a]=1;
                status[Pz][b-1][a+1]=1;
            }
            break;
        case 4:
            if(r_count == 0){
                status[Pz][b][a+1]=1;
                status[Pz][b-1][a]=1;
                status[Pz][b-1][a+1]=1;
            }
            break;
        case 5:
            if(r_count == 0){
                status[Pz][b+2][a]=1;
                status[Pz][b+1][a]=1;
                status[Pz][b-1][a]=1;
            }else if(r_count == 2 || r_count == 4){
                for(int i=0;i<4;i++){
                    int p=Pz+low[i].z;
                    int q=Py+low[i].y;
                    int r=Px+low[i].x;
                    status[p][q][r]=1;
                }
            }
            break;
    }
}

int ***alloc_mat(int zlen, int ylen, int xlen)
{
    int u, v;

    if ((status = malloc(zlen * sizeof(*status))) == NULL) {
        perror("malloc 1");
        return NULL;
    }

    for (u=0; u < zlen; ++u)
        status[u] = NULL;

    for (u=0; u < zlen; ++u)
        if ((status[u] = malloc(ylen * sizeof(*status[u]))) == NULL) {
            perror("malloc 2");
            free_mat(status, zlen, ylen);
            return NULL;
        }

    for (u=0; u < zlen; ++u)
        for (v=0; v < ylen; ++v)
            status[u][v] = NULL;

    for (u=0; u < zlen; ++u)
        for (v=0; v < ylen; ++v)
            if ((status[u][v] = malloc(xlen * sizeof (*status[u][v]))) == NULL) {
                perror("malloc 3");
                free_mat(status, zlen, ylen);
                return NULL;
            }

    return status;
}

void free_mat(int ***status, int zlen, int ylen)
{
    int u, v;

    for (u=0; u < zlen; ++u) {
        if (status[u] != NULL) {
            for (v=0; v < ylen; ++v)
                free(status[u][v]);
            free(status[u]);
        }
    }
    free(status);
}



