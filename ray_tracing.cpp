#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cmath>
using namespace glm;

// global vars
const int WIDTH = 800;
const int HEIGHT = 600;

// functions

// structures and classes :D
class Engine{
public:
    // -- Quad & Texture render
    GLFWwindow* window;
    GLuint quadVAO;
    GLuint texture;
    GLuint shaderProgram;

    Engine(){
        this->window = StartGLFW();
        this->shaderProgram = CreateShaderProgram();
        
        auto result = QuadVAO();
        this->quadVAO = result[0];
        this->texture = result[1];
    }
    GLFWwindow* StartGLFW(){
        if(!glfwInit()){
            std::cerr<<"glfw failed init, PANIC PANIC!"<<std::endl;
            return nullptr;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "ray tracer", NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create GLFW window." << std::endl;
            glfwTerminate();
            return nullptr;
        }
        glfwMakeContextCurrent(window);
        
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW." << std::endl;
            glfwTerminate();
            return nullptr;
        }

        glViewport(0, 0, WIDTH, HEIGHT);
        return window;
    };
    GLuint CreateShaderProgram(){
        const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        })";

        const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoord);
        })";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);
        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
        }

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    };

    std::vector<GLuint> QuadVAO(){
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,  // top left
            -1.0f, -1.0f,  0.0f, 0.0f,  // bottom left
            1.0f, -1.0f,  1.0f, 0.0f,  // bottom right

            -1.0f,  1.0f,  0.0f, 1.0f,  // top left
            1.0f, -1.0f,  1.0f, 0.0f,  // bottom right
            1.0f,  1.0f,  1.0f, 1.0f   // top right

        };
        
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::vector<GLuint> VAOtexture = {VAO, texture};
        return VAOtexture;
    }
    void renderScene(std::vector<unsigned char> pixels) {
        // update texture w/ ray-tracing results
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, 
                    GL_UNSIGNED_BYTE, pixels.data());

        // clear screen and draw textured quad
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        GLint textureLocation = glGetUniformLocation(shaderProgram, "screenTexture");
        glUniform1i(textureLocation, 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    };
};

struct Ray{
    vec3 direction;
    vec3 origin;
    Ray(vec3 o, vec3 d) : origin(o), direction(normalize(d)){}
};
struct Material{
    vec3 color;
    float specular;
    float emission;
    Material(vec3 c, float s, float e) : color(c), specular(s), emission(e) {}
};
struct Object{
    vec3 centre;
    float radius;
    Material material;

    Object(vec3 c, float r, Material m) : centre(c), radius(r), material(m) {}

    bool Intersect(Ray &ray, float &t){
        vec3 oc = ray.origin - centre;
        float a = glm::dot(ray.direction, ray.direction); // ray direction scale by t
        float b = 2.0f * glm::dot(oc, ray.direction);     // 
        float c = glm::dot(oc, oc) - radius * radius;     // adjustment by sphere radius
        double discriminant = b*b - 4*a*c;
        if(discriminant < 0){return false;}          // no intersection with sphere

        float intercept = (-b - sqrt(discriminant)) / (2.0f*a);
        if(intercept < 0){
            intercept = (-b + sqrt(discriminant)) / (2.0f*a);
            if(intercept<0){return false;}           // intersection is behind origin
        }
        t = intercept;
        return true;
    };

    vec3 getNormal(vec3 &point) const{
        return normalize(point - centre);
    }
};

class Scene {
public:
    std::vector<Object> objs;
    vec3 lightPos;
    Scene() : lightPos(5.0f, 5.0f, 5.0f) {}

    vec3 trace(Ray &ray){
        float closest = INFINITY;
        const Object* hitObj = nullptr;

        for(auto& obj : objs){
            float t;                    // distance to intersection
            if(obj.Intersect(ray, t)){
                if(t < closest) {
                    closest = t;
                    hitObj = &obj;
                }
            }
        };
        if(hitObj){
            vec3 hitPoint = ray.origin + ray.direction * closest;     // point on obj hit by ray
            vec3 normal = hitObj->getNormal(hitPoint);
            vec3 lightDir = normalize(lightPos - hitPoint);          // direction light to hitpoint

            float diff = std::max(glm::dot(normal, lightDir), 0.0f); // diffuse lighting

            Ray shadowRay(hitPoint + normal * 0.001f, lightDir); // slightly up to avoid errors ;P
            // check if is in shadow
            bool inShadow = false;
            
            // Actually check for shadows by testing if any object blocks light
            for(auto& obj : objs) {
                float t;
                if(obj.Intersect(shadowRay, t)) {
                    inShadow = true;
                    break;
                }
            }

            vec3 color = hitObj->material.color;
            float ambient = 0.1f; // minimum light level

            if (inShadow) {
                return color * ambient;
            }

            return color * (ambient + diff * 0.9f);
        }

        return vec3(0.0f, 0.0f, 0.1f); 
    }
};


// --- main loop ---- //
int main(){
    Engine engine;
    Scene scene;

    // Add a black hole (large dark sphere at the origin)
    Object blackhole(vec3(0.0f, 0.0f, -10.0f), 2.5f, Material(vec3(0.02f, 0.02f, 0.05f), 0.0f, 0.0f));
    // Add a sun (bright yellow sphere, starts far away)
    Object sun(vec3(0.0f, 0.0f, 10.0f), 1.5f, Material(vec3(1.0f, 0.9f, 0.2f), 0.2f, 0.0f));

    // Add some static objects for scene variety
    scene.objs = {
        blackhole,
        sun,
        Object(vec3(3.0f, 0.0f, -7.0f), 1.5f, Material(vec3(0.2f, 1.0f, 0.2f), 0.5f, 0.0f))
    };

    std::vector<unsigned char> pixels(WIDTH * HEIGHT * 3);

    float sunSpeed = 0.05f; // units per frame
    bool sunFalling = true;

    while(!glfwWindowShouldClose(engine.window)){
        glClear(GL_COLOR_BUFFER_BIT);

        // Animate the sun: move it toward the black hole
        if (sunFalling) {
            // Move sun along -z axis
            scene.objs[1].centre.z -= sunSpeed;
            // If sun reaches the black hole, stop it (simulate "falling in")
            float dist = glm::length(scene.objs[1].centre - scene.objs[0].centre);
            if (dist < scene.objs[0].radius + scene.objs[1].radius * 0.7f) {
                sunFalling = false;
                // Optionally, shrink the sun to simulate being "swallowed"
                scene.objs[1].radius *= 0.95f;
                // Fade out the sun color
                scene.objs[1].material.color *= 0.95f;
                // If sun is very small, hide it
                if (scene.objs[1].radius < 0.1f)
                    scene.objs[1].material.color = vec3(0.0f);
            }
        }

        for(int y = 0; y < HEIGHT; ++y){
            for(int x = 0; x < WIDTH; ++x){
                float aspectRatio = float(WIDTH) / float(HEIGHT);
                float u = float(x) / float(WIDTH);
                float v = float(y) / float(HEIGHT);

                vec3 direction(
                    (2.0f * u - 1.0f) * aspectRatio,
                    -(2.0f * v - 1.0f),
                    -1.0f
                );
                Ray ray(vec3(0.0f, 0.0f, 0.0f), normalize(direction));
                vec3 color = scene.trace(ray);

                int index = (y * WIDTH + x) * 3;
                pixels[index + 0] = static_cast<unsigned char>(glm::clamp(color.r, 0.0f, 1.0f) * 255);
                pixels[index + 1] = static_cast<unsigned char>(glm::clamp(color.g, 0.0f, 1.0f) * 255);
                pixels[index + 2] = static_cast<unsigned char>(glm::clamp(color.b, 0.0f, 1.0f) * 255);
            }
        }
        
        engine.renderScene(pixels);
    }

    glfwTerminate();
}


// func dec's





