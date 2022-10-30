#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>

int glVboDrawIndex = 0;
bool glVboON;

void glBegin(unsigned int mode, bool vbo){ 
    glVboDrawIndex = 0;
    if (!vbo)
        glBegin(mode);
}

void glVertex2f(float x, float y, float* list, bool vbo){
    if (glVboON){
        list[glVboDrawIndex] = x;
        list[glVboDrawIndex + 1] = y;

        glVboDrawIndex += 2;
    }
    else
        glVertex2f(x, y);
}

struct GLPOINT{
    float x, y;
};

struct vboS{
    unsigned int vbo;
    float* list;
};

GLPOINT glLowestPoint(float* list, int size){
    GLPOINT lowest = {list[0], list[1]};
    
    for (int y = 0; y < 2; y++)
        for (int i = 0; i < size; i++){
            if (!((float)(i/2) - (int)(i/2)) && (y || list[i] < lowest.y)){
                if (!y)
                    lowest.y = list[i];
                else 
                    list[i] -= lowest.y;
            }

            else if ((float)(i/2) - (int)(i/2) && (y || list[i] < lowest.x)){
                if (!y) 
                    lowest.x = list[i];
                else
                    list[i] -= lowest.x;
            }
        }
    
    return lowest;
}


void glEnd(unsigned int mode, float* list, int size, unsigned int* vbo){
    if (glVboON){
        GLPOINT lowest = glLowestPoint(list, size);

        if (*vbo == -1){
            float vblist[10];
            
            for (int i = 0; i < size; i++)
                vblist[i] = list[i];   

            glGenBuffers(1, vbo);
            glBindBuffer(GL_ARRAY_BUFFER, *vbo);

            glBufferData(GL_ARRAY_BUFFER, sizeof(vblist), vblist, GL_STATIC_DRAW);
        }

        
        glPushMatrix();
        glTranslatef(lowest.x, lowest.y, 0);

        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, *vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Draw the triangle !
        glDrawArrays(GL_POLYGON, 0, 5); // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDisableVertexAttribArray(0);
        glPopMatrix();
    }
    else
        glEnd();
}

void glFreeVBO(unsigned int vbo){
    glDeleteBuffers(1, &vbo);
}

#ifdef GL_VECTOR_VBO
#ifndef RSGL_H
namespace RSGL{
    struct point{  
        bool operator==(point p){ return x == p.x && y == p.y; } //!< if one point == another point (same values)
        int x /*!< x*/, y /*!< y*/; 
    }; //!< a single points

    struct rect : point{ int w, h;
        bool operator==(rect r){ return x == r.x && y == r.y && w == r.w && h == r.h; } //!< if one rect == another rect (same values)
        rect(){}
        rect(int X, int Y, int W, int H){x = X; y = Y; w = W; h = H;} //!< load with just ints
        rect(RSGL::point p){x = p.x; y = p.y;} //!< load with RSGL::point
        rect(RSGL::area a){w = a.w; h = a.h;} //!< load with RSGL::area
    }; //!< Rect is RSGL::point combined with RSGL::area
    typedef rect oval; //!< oval is the same thing as a rect struct

    struct circle : point { //!< circle as a child of point 
        bool operator==(circle c){ return x == c.x && y == c.y && d == c.d; } //!< if one circle == another circle (same values)
        int d;  //!< diameter of the circle (radius * 2)
        circle(){}
        circle(int X, int Y, int D){x = X; y = Y; d = D;} //!< load circle with circle values
    }; //!< circle struct (made of x, y and d)

    typedef std::array<RSGL::point, 3> triangle; //!< Triangle type
    enum shape{ RECT, LINE, CIRCLE, IMAGE, TRIANGLE, TEXT, EMPTY }; //!< shape constants
};
#endif

#include <vector>
struct VBO{ unsigned int vbo; RSGL::shape s; RSGL::triangle t; RSGL:: rect r; RSGL::circle c;  };
std::vector<VBO> gl_vbos;

void glEnd(unsigned int m, VBO vbo, float* data){
    unsigned int vbobuffer = -1;
    bool sentNegBuff = true, isEqual = true;

    for (auto& vb : gl_vbos){
        if (vb.s == vbo.s && (RSGL::area)vb.r == vbo.r && vb.c.d == vbo.c.d && (vbo.t[0] == vb.t[0] && vbo.t[1] == vb.t[2] && vbo.t[3] == vb.t[3])){
            sentNegBuff = false;
            vbobuffer = vb.vbo;
        }
    }

    glEnd(m, data, sizeof(data)+2, &vbobuffer); 

    if (sentNegBuff){
        vbo.vbo = vbobuffer;
        gl_vbos.insert(gl_vbos.end(), vbo);
    }
}

void glFreeVBOBuffer(){
    for (auto& vb : gl_vbos)
        glDeleteBuffers(1, &vb.vbo);
}

#endif
