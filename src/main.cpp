#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "shader.h"
#include "mytypes.h"

#define MAX_SIERPINSKI_DEPTH 8      //change this to allow for more recursive depth for
                                    // Sierpinski's Triangle

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void addSierpinskiPts(point_t a, point_t b, point_t c, int depth, std::vector<float>& points);
void initSierpinski(std::vector<float> &points);
void sierpinskiOpenGLObj(unsigned int &VAO, unsigned int &VBO);
void backgroundOpenGLObj(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO);
GLFWwindow* glfwOpenGLInit();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


/**
 * Entry point of the program, also where the main logic of rendering takes place
*/
int main()
{
    GLFWwindow *window = glfwOpenGLInit();
    if(window == NULL){
        return -1;
    }

    Shader myShader("VertexShader.glsl", "FragmentShader.glsl");

    unsigned int triVAO, triVBO;
    sierpinskiOpenGLObj(triVAO, triVBO);


    //VAOs, VBOs, EBOs for the background
    unsigned int bgVAO, bgVBO, bgEBO;
    backgroundOpenGLObj(bgVAO, bgVBO, bgEBO);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);   //uncomment to draw in wireframe mode
    //---------------RENDER LOOP-------------------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        //fill background
        // -----------------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //use shader programs defined in shader.h
        myShader.use();
        //render background rectangle
        glBindVertexArray(bgVAO);
        glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //render sierpinski triangle
        glBindVertexArray(triVAO);
        int triBufSize = 0;
        glBindBuffer(GL_ARRAY_BUFFER, triVBO);
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &triBufSize);
        glDrawArrays(GL_TRIANGLES, 0, triBufSize);
        glBindVertexArray(0);
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //---------------FINISHED RENDER LOOP-------------------

    //finished rendering, deallocate resources
    glDeleteVertexArrays(1, &triVAO);
    glDeleteBuffers(1, &triVBO);
    glDeleteVertexArrays(1, &bgVAO);
    glDeleteBuffers(1, &bgVBO);
    glDeleteBuffers(1, &bgEBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

/**
 * Method that gives the initial pts of a sierpinski triangle, then calls the recursive
 * function to add necessary vertices to a vector of floats
 * @param points the vector we wish to add vertex coordinates to
 * post: points contains the vertex information of a Sierpinski Triangle
*/
void initSierpinski(std::vector<float> &points){
    point_t a = {-0.5f, -0.5f,  0.0f};
    point_t b = { 0.0f,  0.5f,  0.0f};
    point_t c = { 0.5f, -0.5f,  0.0f};
    addSierpinskiPts(a, b, c, 0, points);
}


/**
 * Helper method to calculate midpoint between two coordinates in 3d space
 * @param a the first point
 * @param b the second point
 * @return a point that contains the x, y, z midpoint coordinates between
 *         'a' and 'b'
*/
point_t midpoint(point_t a, point_t b){
    point_t p;
    p.x = (a.x + b.x) / 2.0;
    p.y = (a.y + b.y) / 2.0;
    p.z = (a.z + b.z) / 2.0;
    return p;
}

/**
 * Helper function that adds all the position and color values of a vertex into a vector of floats
 * @param p a point that contains the x, y, z values that we wish to add to the vector
 * @param depth the current recursive depth of the point for which we wish to draw
 *              we will use this value to determine what the blue color of the vertex should be
 * @param points a reference to a vector to which we add the necessary vertex data (color and position)
 * pre: none
 * post: points contains the needed position and color data
*/
void addPosAndColor(point_t p, int depth, std::vector<float> &points){
        points.push_back(p.x);
        points.push_back(p.y);
        points.push_back(p.z);
        points.push_back(0.25f);
        points.push_back(depth * 1.0/MAX_SIERPINSKI_DEPTH);
        points.push_back(0.75f);
}
/**
 * Recursively puts the info needed to draw a sierpinski triangle into
 * a vector
 * @param a a 3d point representing one of the vertices of a triangle
 * @param b another vertex of same triangle
 * @param c another vertex of same triangle
 * @param depth the current recursive depth
 * @param points the vector we want to add the points to
 * 
 * post: points should have the points of a sierpinski triangle, with the smallest recursive triangle 
 *       held at the end of the vector
 */
void addSierpinskiPts(point_t a, point_t b, point_t c, int depth, std::vector<float>& points){
    if(depth <= MAX_SIERPINSKI_DEPTH){
        addPosAndColor(a, depth, points);
        addPosAndColor(b, depth, points);
        addPosAndColor(c, depth, points);

        point_t ab = midpoint(a, b);
        point_t ac = midpoint(a, c);
        point_t bc = midpoint(b, c);
        addSierpinskiPts(a, ab, ac, depth + 1, points);
        addSierpinskiPts(b, ab, bc, depth + 1, points);
        addSierpinskiPts(c, ac, bc, depth + 1, points);
    }
}


/**
 * Process all user input by querying GLFW whether relevant
 * keys are pressed or released during the current frame, and
 * react accordingly.
 * For our current implementation, close the window if the user
 * presses the escape key.
 * @param window the window for which we want to focus on keypresses
 * pre: window is a valid window
 * post: close the window if the user presses the escape key
*/
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}

/**
 * glfw: whenever the window size changed (by OS or user resize) this callback function executes
 * @param window the window that is being resized
 * @param width the width to resize to
 * @param height the height to resize to
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

/**
 * Sets up a GLFW Window and loads the OpenGL function pointers
 * @return NULL if we failed to initialize the GLFW window or the function pointers
 *          otherwise, return a GLFWwindow pointer to the initialized window
 * post: - OpenGL function pointers are properly initialized
 *       - return a pointer to the GLFWwindow that has been initialized
*/
GLFWwindow* glfwOpenGLInit(){
// glfw: initialize to context version 3.3
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to init window\n");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to load all OpenGL function pointers\n");
        glfwTerminate();
        return NULL;
    }
    return window;    
}

/**
 * Generates a vertex array object for the VAO and a Vertex Buffer object for the VBO. 
 * Then loads the vertices of the sierpinski triangle into the VBO.
 * @param VAO a reference to a VAO id
 * @param VBO a reference to a VBO id
 * post: - VAO is associated with VBO being bound to GL_ARRAY_BUFFER
 *       - the vertices of a sierpinski triangle are located in Vertex Buffer Object who's
 *         ID is given by VBO
 *       - VAOs and VBOs are unbound
 *       
*/
void sierpinskiOpenGLObj(unsigned int &VAO, unsigned int &VBO){
    //---------FILLING VERTICES WITH POINTS OF SIERPINSKI TRIANGLE-----------
    std::vector<float> points = {};
    initSierpinski(points);
    const size_t len = points.size();
    float vertices[len];
    //add all elements into array so we can put it in the VBO
    for(int i = 0; i < len; i++){
        vertices[i] = points.at(i);
    }
    //---------FILLING VERTICES WITH POINTS OF SIERPINSKI TRIANGLE-----------

    //VAOs and VBOs for sierpinski triangle
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //assigning first attribute (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //assign second attribute (color);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);       //unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);   //safely unbind VBO without dissasociating it from VAO
}


/**
 * Generates VAOs/VBOs/EBOs for the background rectangle
 * @param VAO a reference to an ID of a Vertex Array Object
 * @param VBO  a reference to an ID of a Vertex Buffer Object
 * @param EBO a reference to an ID of a Element Buffer Object
 * post: - VAO is associated with the element/object buffers of VBO and EBO
 *       - EBO is bound to a GL_ELEMENT_ARRAY_BUFFER that contains
 *         the draw order of the rectangle
 *       - VBO is bound to a GL_ARRAY_BUFFER that contains the vertices
 *         of the rectangle
 * 
*/
void backgroundOpenGLObj(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO){
    //--------VERTICES FOR THE BACKGROUND----------------
    float rect[] = {
        //position              //color
        -1.0f, -1.0f,  0.5f,    0.0f,  0.0f,  0.5f,     //bottom left
        -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  0.125f,   //top left
         1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  0.125f,   //top right
         1.0f, -1.0f,  0.5f,    1.0f,  1.0f,  0.5f      //bottom right
    };
    unsigned int drawOrder[] = {
        0, 1, 2,    //triangle one (top left)
        0, 2, 3     //triangle two (bottom right)
    };
    //--------VERTICES FOR THE BACKGROUND----------------

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(drawOrder), drawOrder, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);       //unbind VAO 1st to avoid dissociating bound array/element buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);   //safely unbind VBOs w/o dissociating from VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);   //safely unbind EBOs w/o dissociating from VAO
}
