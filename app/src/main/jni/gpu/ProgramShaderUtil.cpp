//
// Created on 2017/10/25.
//
#include "ProgramShaderUtil.h"
#include "Logger.h"

GLuint ProgramShaderUtil::loadShader(GLenum type, const GLchar *string){
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader ( type );
    if ( shader == 0 )
        return 0;

    // Load the shader source
    glShaderSource ( shader, 1, &string, NULL );

    // Compile the shader
    glCompileShader ( shader );

    // Check the compile status
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

    glReleaseShaderCompiler();

    if ( !compiled )
    {
        GLint infoLen = 0;

        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = new char[infoLen];

            glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
            LOGE ( "Error compiling shader:\n%s\n", infoLog );

            delete[]infoLog;
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;
}

GLuint ProgramShaderUtil::createProgram(const GLchar* vertexSource, const GLchar* fragmentSource){
        int vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            return 0;
        }
        int pixelShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (pixelShader == 0) {
            glDeleteShader(vertexShader);
            return 0;
        }

        GLuint program = glCreateProgram();
        if (program != 0) {
            glAttachShader(program, vertexShader);
            glAttachShader(program, pixelShader);
            glLinkProgram(program);
            GLint linkStat;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStat);
            if (!linkStat) {
                LOGE("Could not link program: ");
                GLint infoLen = 0;
                glGetProgramiv ( program, GL_INFO_LOG_LENGTH, &infoLen );
                if ( infoLen > 1 ){
                    char* infoLog =new char[infoLen];
                    glGetProgramInfoLog ( program, infoLen, NULL, infoLog );
                    LOGE ( "Error linking program:\n%s\n", infoLog );
                    delete[]infoLog;
                }
            glDeleteProgram(program);
            program = 0;
           }
        }
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);
        return program;
}
