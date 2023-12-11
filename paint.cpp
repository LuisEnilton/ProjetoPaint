/*
 * Computacao Grafica
 * Codigo Exemplo: Rasterizacao de Segmentos de Reta com GLUT/OpenGL
 * Autor: Prof. Laurindo de Sousa Britto Neto
 */

// Bibliotecas utilizadas pelo OpenGL
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <cmath>
#include <cstdlib>
#include <stack>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <forward_list>
#include "glut_text.h"

using namespace std;
#define ESC 27

struct Vertice{
	int x;
	int y;
	
	Vertice(int _x, int _y){
		this->x = _x;
		this->y = _y;
	}
	
	Vertice(){
		
	}
};

struct Forma{
	int tipo;
	forward_list<Vertice> lista_vertices;
};
struct Cor {
    float r, g, b;  // RGB color components
};

int width = 1200;
int height = 900;
int qtd = 0; // os primeiros qtd poligonos são pintados
// Marca os pixels já visitados para o algoritmo floodfill


// Função para verificar se duas cores são iguais

forward_list<Forma> lista_formas;

enum formType{LIN = 1, TRI = 2, RET = 3, POL = 4, CIR = 5 , FLOOD = 6,TRA = 7, ESL = 8, CIS = 9, REF = 10, ROT = 11};

int mode = LIN;
bool click1 = false;
bool click2 = false;
bool poligono = false;
int x_m, y_m;
int x_1, y_1, x_2, y_2;
int x_origem,y_origem; // coordenadas de inicio do poligono
int x_tri[3];
int y_tri[3];





pair<float,float> calcularCentroide(forward_list<Vertice>& pontos) {
    pair<float,float> centroide = {0.0, 0.0};
    int numPontos = 0;
	
    for (auto it = pontos.begin(); it != pontos.end(); ++it) {
        centroide.first += it->x;
        centroide.second += it->y;
        numPontos++;
	}

    if (numPontos > 0) {
        centroide.first = round(centroide.first/numPontos);
        centroide.second = round(centroide.second/numPontos);
    }

    return centroide;
}

void pushForma(int tipo){
    Forma f;
    f.tipo = tipo;
    lista_formas.push_front(f);
}




void pushVertice(int x, int y){
    Vertice v;
    v.x = x;
    v.y = y;
    lista_formas.front().lista_vertices.push_front(v);
}

void Linha(int x1, int y1, int x2, int y2){
    pushForma(LIN);
    pushVertice(x1, y1);
    pushVertice(x2, y2);
}

void Triangulo(int x[], int y[]){
	pushForma(TRI);
	pushVertice(x[0], y[0]);
    pushVertice(x[1], y[1]);
    pushVertice(x[2], y[2]);
}

void Retangulo(int x1, int y1, int x2, int y2){
	pushForma(RET);
	pushVertice(x1, y1);
	pushVertice(x2, y2);
}

void Circulo(int x1, int y1, int x2, int y2){
	pushForma(CIR);
	pushVertice(x2, y2);
	pushVertice(x1, y1);
}



void init(void);
void reshape(int w, int h);
void display(void);
void menu_popup(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mousePassiveMotion(int x, int y);
void drawPixel(int x, int y);
void desenhaFormas();
void Linhabresenham(double x1,double y1,double x2,double y2);
void CirculoBresenham(double x1, double y1, double x2, double y2);
void translacao(int dx, int dy);
void escala(float sx, float sy);
void cisalhamento(float shx, float shy);
void reflexao(bool horizontal, bool vertical);
void rotacao(float angle);
void floodFill(int x, int y, Cor old_col, Cor new_col);
void scanlineFill(int y, int x1, int y1, int x2, int y2);
void paintPolygon(Forma& polygon);
int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Projeto Paint");
	
	init();
	
	glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mousePassiveMotion);
    
    glutCreateMenu(menu_popup);
    glutAddMenuEntry("Linha", LIN);
    glutAddMenuEntry("Triangulo", TRI);
    glutAddMenuEntry("Retangulo", RET);
	glutAddMenuEntry("Poligono", POL);
	glutAddMenuEntry("Circulo", CIR);
	glutAddMenuEntry("Pintar", FLOOD);
	glutAddMenuEntry("Translacao", TRA);
    glutAddMenuEntry("Escala", ESL);
    glutAddMenuEntry("Cisalhamento", CIS);
    glutAddMenuEntry("Reflexao", REF);
    glutAddMenuEntry("Rotacao", ROT);
    glutAddMenuEntry("Sair", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	return EXIT_SUCCESS;
}

void init(void){
    glClearColor(1.0, 1.0, 1.0, 1.0); //Limpa a tela com a cor branca;
}

void reshape(int w, int h){
	// Muda para o modo de projecao e reinicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Definindo o Viewport para o tamanho da janela
	glViewport(0, 0, w, h);
	
	width = w;
	height = h;
    glOrtho (0, w, 0, h, -1 ,1);  

   // muda para o modo de desenho
	glMatrixMode(GL_MODELVIEW);
 	glLoadIdentity();

}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT); //Limpa o buffer de cores e reinicia a matriz
    glColor3f (0.0, 0.0, 0.0); // Seleciona a cor default como preto
    desenhaFormas(); // Desenha as s geometricas da lista
    //Desenha texto com as coordenadas da posicao do mouse
    draw_text_stroke(0, 0, "(" + to_string(x_m) + "," + to_string(y_m) + ")", 0.2);
    glutSwapBuffers(); // manda o OpenGl renderizar as primitivas
}

void menu_popup(int value){
	 if (value == 0) exit(EXIT_SUCCESS);
    switch (value){
		case 6 : 
			qtd = distance(lista_formas.begin(), lista_formas.end());
			break;
		case 7: translacao(20, 20); break;
		case 8: escala(0.5, 0.5); break;
		case 9: cisalhamento(0.7, 0); break;
		case 10: reflexao(false, true); break;
		case 11: rotacao(45); break;
	}  
    mode = value;
}

void keyboard(unsigned char key, int x, int y){
    switch (key){
        case ESC: exit(EXIT_SUCCESS); break;
    
		case 'p':
            // Verifica se está desenhando um polígono
            if (poligono) {
                poligono = false;  // Define o estado do desenho do polígono como concluído
                x_1 = 0;
                y_1 = 0;
                click1 = false;
                glutPostRedisplay();
            }
            break;
        case 'a':
        	translacao(-20,0);
			break;
		case 'd':
        	translacao(20,0);
			break;
		case 'w':
			translacao(0,20);
			break;
		case 's':
			translacao(0,-20);
			break;
    }

    
    	
}

void mouse(int button, int state, int x, int y){
    switch (button) {
        case GLUT_LEFT_BUTTON:
            switch(mode){
                case LIN:
                    if(state == GLUT_DOWN){
                        if(click1){
                            x_2 = x;
                            y_2 = height - y - 1;
                            Linha(x_1, y_1, x_2, y_2);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                        }
                    }
                	break;
                
                case TRI:
                	if(state == GLUT_DOWN){
                        if(click1){
                        	x_2 = x;
                        	y_2 = height - y - 1;
                            x_tri[1] = x_2;
                            y_tri[1] = y_2;
                            click1 = false;
                            click2 = true;
                            glutPostRedisplay();
                        }
                        else if(click2){
							x_tri[2] = x;
							y_tri[2] = height - y - 1;
							Triangulo(x_tri, y_tri);
							click2 = false;
							glutPostRedisplay();
						}
						else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                            x_tri[0] = x_1;
                            y_tri[0] = y_1;
                        }
                    }
                	break;
                
                case RET:
                    if(state == GLUT_DOWN){
                        if(click1){
                            x_2 = x;
                            y_2 = height - y - 1;
                            Retangulo(x_1, y_1, x_2, y_2);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                        }
                    }
                	break;
                
                case CIR:
                	if(state == GLUT_DOWN){
						if(click1){
							x_2 = x;
							y_2 = height - y - 1;
							Circulo(x_1, y_1, x_2, y_2);
							click1 = false;
							glutPostRedisplay();
						}
						//centro
						else{
							click1 = true;
							x_1 = x;
							y_1 = height - y -1;
						}
					}
					break;
				case POL:
					if(state == GLUT_DOWN){
						if(!click1  ){
                            click1 = true;
                            x_1 = x_2 =x;
                            y_1 = y_2 =height - y - 1;
                            x_origem = x_1;
							y_origem = y_1;
                            poligono = true;
                            pushForma(POL);
                            pushVertice(x_1,y_1);
                        }else{
                            swap(x_1,x_2);
                            swap(y_1,y_2);
							x_2 = x;
                            y_2 = height - y - 1;
							pushVertice(x_2,y_2);
							glutPostRedisplay();
                        }
					}
					break;
				
            }
 			break;
    }
}

void mousePassiveMotion(int x, int y){
    x_m = x; y_m = height - y - 1;
    glutPostRedisplay();
}

void drawPixel(int x, int y){
	
	glBegin(GL_POINTS);
		glVertex2i(x, y);
	glEnd();	
}

void drawPixel2(int x, int y){
	glColor3f(1, 0, 0);
	glBegin(GL_POINTS);
		glVertex2i(x, y);
	glEnd();	
}

void inserirCor(Forma& polygon){
		Forma f;
		f.tipo = RET;
		int x[2];
		int y[2];
		int i = 0;
		for(auto v = polygon.lista_vertices.begin(); v != polygon.lista_vertices.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
		}
		Vertice v1(x[0],y[0]);
		Vertice v2(x[0],y[1]); 
		Vertice v3(x[1],y[1]); 
		Vertice v4(x[1],y[0]);  
		f.lista_vertices.push_front(v1);
		f.lista_vertices.push_front(v2);
		f.lista_vertices.push_front(v3);
		f.lista_vertices.push_front(v4);
		paintPolygon(f);
	
}


void desenhaFormas(){
    //Apos o primeiro clique, desenha a reta com a posicao atual do mouse
    if(click1){
    	switch (mode){
			case LIN:
				Linhabresenham(x_1, y_1, x_m, y_m);
				break;
			case TRI:
				Linhabresenham(x_1, y_1, x_m, y_m);
				break;
			case RET:
        		Linhabresenham(x_1, y_1, x_m, y_1);
                Linhabresenham(x_m, y_1, x_m, y_m);
                Linhabresenham(x_m, y_m, x_1, y_m);
                Linhabresenham(x_1, y_m, x_1, y_1);
                break;
            case CIR:
            	break;
            case POL:
            	Linhabresenham(x_1,y_1,x_2,y_2);
            	Linhabresenham(x_2,y_2,x_m,y_m);
            	break;
        	
		
		}
	}
	else if(click2){
		switch(mode){
			case TRI:
				Linhabresenham(x_1, y_1, x_2, y_2);
				Linhabresenham(x_2, y_2, x_m, y_m);
				Linhabresenham(x_m, y_m, x_1, y_1);
				break;
		}
	}
    
    //Percorre a lista de s geometricas para desenhar
    int cnt = 0;
    int sz = distance(lista_formas.begin(),lista_formas.end());
    int val = sz - qtd;
	for(forward_list<Forma>::iterator f = lista_formas.begin(); f != lista_formas.end(); f++, cnt++){
    	bool last = f == lista_formas.begin();
		int i = 0, x[3], y[3];
        switch (f->tipo){
            case LIN:
                //Percorre a lista de vertices da  linha para desenhar
                for(auto v = f->lista_vertices.begin(); v != f->lista_vertices.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
                }
                //Desenha o segmento de reta apos dois cliques
                Linhabresenham(x[0], y[0], x[1], y[1]);
  				break;
  			
			case TRI:
   				for(auto v = f->lista_vertices.begin(); v != f->lista_vertices.end(); v++, i++){
					x[i] = v->x;
   					y[i] = v->y;
				}
  				Linhabresenham(x[0], y[0], x[1], y[1]);
        		Linhabresenham(x[1], y[1], x[2], y[2]);
        		Linhabresenham(x[2], y[2], x[0], y[0]);
        		if(cnt >= val)
					paintPolygon(*f);
    			break;
    			
            case RET:
                //Percorre a lista de vertices da  retangulo para desenhar
                for(auto v = f->lista_vertices.begin(); v != f->lista_vertices.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
                }
				//Desenha o segmento de reta apos dois cliques
                Linhabresenham(x[0], y[0], x[1], y[0]);
                Linhabresenham(x[1], y[0], x[1], y[1]);
                Linhabresenham(x[1], y[1], x[0], y[1]);
                Linhabresenham(x[0], y[1], x[0], y[0]);
                if(cnt >= val)
					inserirCor(*f);
                break;
            
            case CIR:
                //Percorre a lista de vertices da  retangulo para desenhar
                for(auto v = f->lista_vertices.begin(); v != f->lista_vertices.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
                }
				//Desenha o segmento de reta apos dois cliques
				glColor3f(0, 0, 0);
				CirculoBresenham(x[0], y[0], x[1], y[1]);
                break;
            case POL:
  			  	auto inicio = f->lista_vertices.begin();
				auto final = f->lista_vertices.end();
				auto aux = inicio;  
				while(aux!= final){
					auto proximo = next(aux);
					if(proximo == final){  
						if(poligono && last) break;
						proximo  = inicio;
					}
					Linhabresenham(aux->x,aux->y,proximo->x,proximo->y);
					aux++;
				}
				if(!(last && poligono) )
					if(cnt >= val)
						paintPolygon(*f);
				break;
			}
	}
	
}






void Linhabresenham(double x1, double y1, double x2, double y2){
	double d, incE, incNE;
	bool S = false;
	bool D = false;
	
	double dx = (x2 - x1);
	double dy = (y2 - y1);
	
	
	if((dx * dy) < 0){
		y1 = -y1;
		y2 = -y2;
		dy = (y2 - y1);
		
		S = true;
	}
	if(fabs(dx) < fabs(dy)){
		swap(x1,y1);
		
		swap(x2,y2);
		
		dx = (x2 - x1);
		dy = (y2 - y1);
		
		D = true;
	}
	if(x1 > x2){
		swap(x1,x2);
		
		swap(y1,y2);
		
		dx = (x2 - x1);
		dy = (y2 - y1);
	}
	
	int y = int(y1);
	d = 2*dy - dx;
	incE = 2*dy;
	incNE = 2*dy - 2*dx;
	
	drawPixel(int(x1), int(y1));
	for(int x = int(x1)+1; x< int(x2); x++){
		if (d <= 0){
			d = d + incE;
		}
		else{
			d = d + incNE;
			y++;
		}
		
		if(D && S){
			drawPixel(y, -x);
		}
		else if(D){
			drawPixel(y, x);
		}
		else if(S){
			drawPixel(x, -y);
		}
		else{
			drawPixel(x, y);
		}
		
	}
}

void CirculoBresenham(double x1, double y1, double x2, double y2){
	double raio = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

	
	int d = 1 - int(raio);
	int incE = 3;
	int incSE = -2*int(raio) + 5;
	
	drawPixel(x1, y1 + int(raio));
	drawPixel(x1, y1 - int(raio));
	drawPixel(x1 + int(raio), y1);
	drawPixel(x1 - int(raio), y1);
	
	int y = int(raio);
	
	for(int x = 1; y>x; x++){
		if(d<0){
			d = d + incE;
			incE = incE + 2;
			incSE = incSE + 2;
			
			drawPixel(x1 + x, y1 + y);
			drawPixel(x1 + y, y1 + x);
			drawPixel(x1 - x, y1 + y);
			drawPixel(x1 + y, y1 - x);
			drawPixel(x1 + x, y1 - y);
			drawPixel(x1 - y, y1 + x);
			drawPixel(x1 - x, y1 - y);
			drawPixel(x1 - y, y1 - x);
		}
		else{
			d = d + incSE;
			incE = incE + 2;
			incSE = incSE + 4;
			
			drawPixel(x1 + x, y1 + y);
			drawPixel(x1 + y, y1 + x);
			drawPixel(x1 - x, y1 + y);
			drawPixel(x1 + y, y1 - x);
			drawPixel(x1 + x, y1 - y);
			drawPixel(x1 - y, y1 + x);
			drawPixel(x1 - x, y1 - y);
			drawPixel(x1 - y, y1 - x);
			
			y = y - 1;
		}
	}
}




void paintPolygon(Forma& polygon) {
    
    

    

    // Find the bounding box of the polygon
    int minX = width, minY = height, maxX = 0, maxY = 0;
    for (auto it = polygon.lista_vertices.begin();it !=polygon.lista_vertices.end();it++) {
        auto v = *it;
		minX = min(minX, v.x);
        minY = min(minY, v.y);
        maxX = max(maxX, v.x);
        maxY = max(maxY, v.y);
    }

    

    // Iterate through scanlines within the bounding box
    for (int y = minY; y <= maxY; ++y) {
        vector<int> intersections;

        // Find intersections with polygon edges
        auto current = polygon.lista_vertices.begin();
        auto next = current;
        ++next;
        for (; current != polygon.lista_vertices.end(); ++current, ++next) {
            if (next == polygon.lista_vertices.end()) {
                next = polygon.lista_vertices.begin();
            }

            int x1 = current->x, y1 = current->y;
            int x2 = next->x, y2 = next->y;

            if ((y1 <= y && y2 > y) || (y1 > y && y2 <= y)) {
                // Edge crosses the scanline
                int x_intersection = static_cast<int>(x1 + (static_cast<double>(y - y1) / (y2 - y1)) * (x2 - x1));
                intersections.push_back(x_intersection);
            }
        }

        // Sort the intersection points
        sort(intersections.begin(), intersections.end());
		
        // Fill the scanline between pairs of intersections
        for (size_t i = 0; i < intersections.size(); i += 2) {
            int x1 = intersections[i];
            int x2 = (i + 1 < intersections.size()) ? intersections[i + 1] : x1;

            for (int x = x1; x <= x2; ++x) {
				drawPixel2(x, y);
            }
        }
    }

    glutPostRedisplay();
}


void translacao(int dx, int dy) {
    for (auto it_forma = lista_formas.begin(); it_forma != lista_formas.end(); ++it_forma) {
        for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
            it_vertice->x += dx;
            it_vertice->y += dy;
        }
    }
    glutPostRedisplay();
}

void escala(float sx, float sy) {
    for (auto it_forma = lista_formas.begin(); it_forma != lista_formas.end(); ++it_forma) {
        // Encontrar o centro do objeto
        float centro_x = 0.0;
        float centro_y = 0.0;
		auto par = calcularCentroide(it_forma->lista_vertices);
		centro_x = par.first;
		centro_y = par.second;
        
            // Escala em relação ao centro do objeto
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                it_vertice->x = static_cast<int>((it_vertice->x - centro_x) * sx + centro_x);
                it_vertice->y = static_cast<int>((it_vertice->y - centro_y) * sy + centro_y);
            
        }
    }
    glutPostRedisplay();
}





void cisalhamento(float shx, float shy) {
    for (auto it_forma = lista_formas.begin(); it_forma != lista_formas.end(); ++it_forma) {
        float centro_x = 0.0;
        float centro_y = 0.0;
		auto par = calcularCentroide(it_forma->lista_vertices);
		centro_x = par.first;
		centro_y = par.second;

        

            // Aplicar cisalhamento em relação ao centro do objeto
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                int x = it_vertice->x + static_cast<int>(shx * (it_vertice->y - centro_y));
                int y = it_vertice->y + static_cast<int>(shy * (it_vertice->x - centro_x));
                it_vertice->x = x;
                it_vertice->y = y;
            
        	}
    }
    glutPostRedisplay();
}

void reflexao(bool horizontal, bool vertical) {
    int h = (horizontal) ? -1 : 1;
    int v = (vertical) ? -1 : 1;

    for (auto it_forma = lista_formas.begin(); it_forma != lista_formas.end(); ++it_forma) {
        float centro_x = 0.0;
        float centro_y = 0.0;
		auto par = calcularCentroide(it_forma->lista_vertices);
		centro_x = par.first;
		centro_y = par.second;

        

            // Transladar para a origem
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                it_vertice->x -= centro_x;
                it_vertice->y -= centro_y;
            }

            // Aplicar a reflexão
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                it_vertice->x *= h;
                it_vertice->y *= v;
            }

            // Transladar de volta para a posição original
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                it_vertice->x += centro_x;
                it_vertice->y += centro_y;
            }
        
    }

    glutPostRedisplay();
}

void rotacao(float angle) {
    float radians = angle * 3.14159265 / 180.0;

    for (auto it_forma = lista_formas.begin(); it_forma != lista_formas.end(); ++it_forma) {
        // Encontrar o centro do objeto
        float centro_x = 0.0;
        float centro_y = 0.0;

        // Contar o número de vértices
        int numVertices = 0;

        for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
            centro_x += it_vertice->x;
            centro_y += it_vertice->y;
            numVertices++;
        }

        if (numVertices > 0) {
            centro_x /= numVertices;
            centro_y /= numVertices;

            // Transladar para a origem
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                it_vertice->x -= centro_x;
                it_vertice->y -= centro_y;
            }

            // Rotacionar
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                int x = static_cast<int>(it_vertice->x * cos(radians) - it_vertice->y * sin(radians));
                int y = static_cast<int>(it_vertice->x * sin(radians) + it_vertice->y * cos(radians));
                it_vertice->x = x;
                it_vertice->y = y;
            }

            // Transladar de volta para a posição original
            for (auto it_vertice = it_forma->lista_vertices.begin(); it_vertice != it_forma->lista_vertices.end(); ++it_vertice) {
                it_vertice->x += centro_x;
                it_vertice->y += centro_y;
            }
        }
    }

    glutPostRedisplay();
}


