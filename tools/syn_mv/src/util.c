#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void read_in_shader_file(FILE *fp, char** buffer)
{
  if(fp)
  {
    fseek(fp,0L, SEEK_END);
    int file_size = ftell(fp);
    rewind(fp);
    *buffer = calloc(1,file_size+1);
    if(!buffer)
    {
      fclose(fp);
      fprintf(stderr,"load shader: memory alloc fails\n");
      exit(1);
    }

    if(1!=fread(*buffer, file_size,1 ,fp))
    {
      fclose(fp);
      free(*buffer);
      fprintf(stderr,"load shader: entire read fails\n");
      exit(1);
    }
  }
  else
  {
    fprintf(stderr,"load shader: empty shader file!\n");
    exit(1);
  }
}

GLuint load_shaders(const char * vertex_file_path,const char * fragment_file_path)
{

      // Create the shaders
      GLuint vertex_shader_ID = glCreateShader(GL_VERTEX_SHADER);
      GLuint fragment_shader_ID = glCreateShader(GL_FRAGMENT_SHADER);

      // Read the Vertex Shader code from the file
      char* vertex_shader_code;
      FILE *vertex_shader_stream = fopen(vertex_file_path,"rb");
      if(vertex_shader_stream){
        read_in_shader_file(vertex_shader_stream,&vertex_shader_code);
        fclose(vertex_shader_stream);
      }else{
        fprintf(stderr,"load shader: cannot open file: %s\n",vertex_file_path);
        exit(1);
      }

      // Read the Fragment Shader code from the file
      char* fragment_shader_code;
      FILE *fragment_shader_stream = fopen(fragment_file_path,"rb");
      if(fragment_shader_stream){
        read_in_shader_file(fragment_shader_stream,&fragment_shader_code);
        fclose(fragment_shader_stream);
      }else{
        fprintf(stderr,"load shader: cannot open file: %s\n",fragment_file_path);
        exit(1);
      }

      GLint result = GL_FALSE;
      int info_log_length;

      // Compile Vertex Shader
      printf("Compiling shader : %s\n", vertex_file_path);
      const GLchar* vertex_shader_pointer= vertex_shader_code;
      glShaderSource(vertex_shader_ID, 1, &vertex_shader_pointer , NULL);
      glCompileShader(vertex_shader_ID);

      // Check Vertex Shader
      glGetShaderiv(vertex_shader_ID, GL_COMPILE_STATUS, &result);
      glGetShaderiv(vertex_shader_ID, GL_INFO_LOG_LENGTH, &info_log_length);
      if ( info_log_length > 0 ){
        char *vertex_shader_error_message = calloc(1,info_log_length+1);
        glGetShaderInfoLog(vertex_shader_ID, info_log_length, NULL, vertex_shader_error_message);
        printf("%s\n", vertex_shader_error_message);
      }

      // Compile Fragment Shader
      printf("Compiling shader : %s\n", fragment_file_path);
      const GLchar* fragment_shader_pointer= fragment_shader_code;
      glShaderSource(fragment_shader_ID, 1, &fragment_shader_pointer , NULL);
      glCompileShader(fragment_shader_ID);

      // Check Fragment Shader
      glGetShaderiv(fragment_shader_ID, GL_COMPILE_STATUS, &result);
      glGetShaderiv(fragment_shader_ID, GL_INFO_LOG_LENGTH, &info_log_length);
      if ( info_log_length > 0 ){
        char *fragment_shader_error_message = calloc(1,info_log_length+1);
        glGetShaderInfoLog(fragment_shader_ID, info_log_length, NULL, fragment_shader_error_message);
        printf("%s\n", fragment_shader_error_message);
      }

      // Link the program
      printf("Linking program\n");
      GLuint program_ID = glCreateProgram();
      glAttachShader(program_ID, vertex_shader_ID);
      glAttachShader(program_ID, fragment_shader_ID);
      glLinkProgram(program_ID);

      // Check the program
      glGetProgramiv(program_ID, GL_LINK_STATUS, &result);
      glGetProgramiv(program_ID, GL_INFO_LOG_LENGTH, &info_log_length);
      if ( info_log_length > 0 ){
        char *program_error_message = calloc(1,info_log_length+1);
        glGetProgramInfoLog(program_ID, info_log_length, NULL, program_error_message);
        printf("%s\n", program_error_message);
      }

      glDetachShader(program_ID, vertex_shader_ID);
      glDetachShader(program_ID, fragment_shader_ID);

      glDeleteShader(vertex_shader_ID);
      glDeleteShader(fragment_shader_ID);
      return program_ID;
}
