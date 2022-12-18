#include "summer_opengl.h"

struct win32_window_dimensions
{
	int width;
	int height;
};

win32_window_dimensions GetWindowDimensions(HWND window)
{
	RECT clientRect;
	win32_window_dimensions dimensions;
	GetClientRect(window, &clientRect);
	
	dimensions.width = clientRect.right - clientRect.left;
	dimensions.height = clientRect.bottom - clientRect.top;
	return dimensions;
}

static void get_orthographic(float left, float right,
                             float top, float bottom,
                             float zNear, float zFar, float matrix[4][4])
{
	matrix[0][0] = 2 / (right - left);
	matrix[0][1] = 0;
	matrix[0][2] = 0; 
	matrix[0][3] = 0; 
	
	matrix[1][0] = 0; 
	matrix[1][1] = 2 / (top - bottom); 
	matrix[1][2] = 0; 
	matrix[1][3] = 0; 
	
	matrix[2][0] = 0; 
	matrix[2][1] = 0; 
	matrix[2][2] = -2 / (zFar - zNear); 
	matrix[2][3] = 0; 
	
	matrix[3][0] = -(right + left) / (right - left); 
	matrix[3][1] = -(top + bottom) / (top - bottom); 
	matrix[3][2] = -(zFar + zNear) / (zFar - zNear);
	matrix[3][3] = 1; 
}

static void Win32GetOpenGlContext(HDC windowDC)
{
	PIXELFORMATDESCRIPTOR desiredPfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,       // The kind of framebuffer. RGBA or palette.
		32,                  // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		8,
		0,
		0, 0, 0, 0,
		24,                  // Number of bits for the depthbuffer
		8,                   // Number of bits for the stencilbuffer
		0,                   // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	
	int pixelFormat = ChoosePixelFormat(windowDC, &desiredPfd);
    
	PIXELFORMATDESCRIPTOR actualPfd;
	DescribePixelFormat(windowDC, pixelFormat,
                        sizeof(PIXELFORMATDESCRIPTOR),
                        &actualPfd);
	SetPixelFormat(windowDC, pixelFormat, &actualPfd);
    
	HGLRC OpenGLRC = wglCreateContext(windowDC);	
	HGLRC shareContext = 0;
	
	//get a default context
	if(wglMakeCurrent(windowDC, OpenGLRC))
	{
		//use the default context to try to get the real context
		wglCreateContextAttribsARB = OPEN_GL_GET_PROC_ADDRESS(wglCreateContextAttribsARB); 
		if(wglCreateContextAttribsARB)
		{
			int attribsList[] = 
			{
				WGL_CONTEXT_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				0,
			};
			HGLRC NewOpenGLRC = wglCreateContextAttribsARB(windowDC, shareContext, attribsList);
			if(NewOpenGLRC)
			{
				wglDeleteContext(OpenGLRC);
				OpenGLRC = NewOpenGLRC;
			}
		}
	}
    
	//assign the real context
	if(wglMakeCurrent(windowDC, OpenGLRC))
	{
		glEnableVertexAttribArray = OPEN_GL_GET_PROC_ADDRESS(glEnableVertexAttribArray);
		if(!glEnableVertexAttribArray)
		{
			OutputDebugString("glEnableVertexAttribArray() not found");
		}
		glVertexAttribPointer = OPEN_GL_GET_PROC_ADDRESS(glVertexAttribPointer);
		if(!glVertexAttribPointer)
		{
			OutputDebugString("glVertexAttribPointer() not found");
		}
		glDisableVertexAttribArray = OPEN_GL_GET_PROC_ADDRESS(glDisableVertexAttribArray);
		if(!glDisableVertexAttribArray)
		{
			OutputDebugString("glDisableVertexAttribArray() not found");
		}
		glGenBuffers = OPEN_GL_GET_PROC_ADDRESS(glGenBuffers);
		if(!glGenBuffers)
		{
			OutputDebugString("glGenBuffers() not found");
		}
		glBindBuffer = OPEN_GL_GET_PROC_ADDRESS(glBindBuffer);
		if(!glBindBuffer)
		{
			OutputDebugString("glBindBuffer() not found");
		}
		glBufferData = OPEN_GL_GET_PROC_ADDRESS(glBufferData);
		if(!glBufferData)
		{
			OutputDebugString("glBufferData() not found");
		}
        
		glCreateShader = OPEN_GL_GET_PROC_ADDRESS(glCreateShader);
		if(!glCreateShader)
		{
			OutputDebugString("glCreateShader() not found");
		}
		glShaderSource = OPEN_GL_GET_PROC_ADDRESS(glShaderSource);
		if(!glShaderSource)
		{
			OutputDebugString("glShaderSource() not found");
		}
		glCompileShader = OPEN_GL_GET_PROC_ADDRESS(glCompileShader);
		if(!glCompileShader)
		{
			OutputDebugString("glCompileShader() not found");
		}
		glCreateProgram = OPEN_GL_GET_PROC_ADDRESS(glCreateProgram);
		if(!glCreateProgram)
		{
			OutputDebugString("glCreateProgram() not found");
		}
		glAttachShader = OPEN_GL_GET_PROC_ADDRESS(glAttachShader);
		if(!glAttachShader)
		{
			OutputDebugString("glAttachShader() not found");
		}
		glLinkProgram = OPEN_GL_GET_PROC_ADDRESS(glLinkProgram);
		if(!glLinkProgram)
		{
			OutputDebugString("glLinkProgram() not found");
		}
		glGetUniformLocation = OPEN_GL_GET_PROC_ADDRESS(glGetUniformLocation);
		if(!glGetUniformLocation)
		{
			OutputDebugString("glGetUniformLocation() not found");
		}
		glUseProgram = OPEN_GL_GET_PROC_ADDRESS(glUseProgram);
		if(!glUseProgram)
		{
			OutputDebugString("glUseProgram() not found");
		}
		glUniform4f = OPEN_GL_GET_PROC_ADDRESS(glUniform4f);
		if(!glUniform4f)
		{
			OutputDebugString("glUniform4f() not found");
		}
		glUniformMatrix4fv = OPEN_GL_GET_PROC_ADDRESS(glUniformMatrix4fv);
		if(!glUniformMatrix4fv)
		{
			OutputDebugString("glUniformMatrix4fv() not found");
		}
		glGetShaderiv = OPEN_GL_GET_PROC_ADDRESS(glGetShaderiv);
		if(!glGetShaderiv)
		{
			OutputDebugString("glGetShaderiv() not found");
		}
	}
	else
	{
		//something went wrong
		OutputDebugString("wglMakeCurrent Failed");
	}
}

static void ogl_init_shaders(Win32_State *win32State)
{
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	
    char Path[MAX_PATH] = {};
    size_t PathLength = strlen(win32State->ExeFilePath);
    
    Copy(PathLength, win32State->ExeFilePath, Path);
    AppendCString(Path + PathLength, "\\..\\src\\shaders\\vertexshader.vertexshader");
	read_file_result vsFile = win32_read_file_to_ntchar(Path);        
    
    Copy(PathLength, win32State->ExeFilePath, Path);
    AppendCString(Path + PathLength, "\\..\\src\\shaders\\fragmentshader.fragmentshader");
	read_file_result fsFile = win32_read_file_to_ntchar(Path);	
    
	GLint success = 0;
	
	glShaderSource(vertexShaderId, 1, &((char*)vsFile.data), NULL);
	glCompileShader(vertexShaderId);	
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	
	glShaderSource(fragmentShaderId, 1, &((char*)fsFile.data), NULL);
	glCompileShader(fragmentShaderId);
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    
	GLuint programId = glCreateProgram();
	Global_tex_shader.program_id = programId;
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);	
	glLinkProgram(programId);
	
	Global_tex_shader.MVP_location = glGetUniformLocation(programId, "MVP");
    
	win32_free_file_memory(vsFile.data);
	win32_free_file_memory(fsFile.data);
}

static void ogl_init_quad_buffers()
{
	glGenBuffers(1, &GlobalQuadBufferHandle);
	glGenBuffers(1, &GlobalQuadIndexBufferHandle);
    
	//this never changes so it only needs to be set once at startup
	for (uint32 i = 0; i < QUAD_BUFFER_SIZE; i++)
	{
		Global_Index_Buffer[(i*6)] = (i*4);
		Global_Index_Buffer[(i*6) + 1] = (i*4) + 1;
		Global_Index_Buffer[(i*6) + 2] = (i*4) + 2;
		Global_Index_Buffer[(i*6) + 3] = (i*4);
		Global_Index_Buffer[(i*6) + 4] = (i*4) + 2;
		Global_Index_Buffer[(i*6) + 5] = (i*4) + 3;
	}
}

static GLuint ogl_init_texture(uint32 textureWidth, uint32 textureHeight, 
                               uint32 bytesPerPixel, void * textureBytes)
{
	GLuint TextureHandle;
    
	glGenTextures(1, &TextureHandle);
	
	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGBA8, textureWidth, textureHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 textureBytes);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	return TextureHandle;
}

static void open_gl_init(Win32_State *win32State, HWND window)
{
	HDC windowDC = GetDC(window);
	Win32GetOpenGlContext(windowDC);	
    
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
    
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	win32_window_dimensions windowDimensions = GetWindowDimensions(window);
    
	float fW, fH;
	const GLdouble pi = 3.1415926535897932384626433832795;
	float aspect = (float)windowDimensions.width/(float)windowDimensions.height;
	float halfAspect = aspect/2.0f; 
	float zNear = 0.1f;
	float zFar = 100.0f;
	float fov = 45.0f;
	
	fH = (float)tan( fov / 360 * pi ) * zNear;
	fW = fH * aspect;	
	
	glViewport(0, 0, windowDimensions.width, windowDimensions.height);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	get_orthographic(-halfAspect, halfAspect, 0.5f, -0.5f, zNear, zFar, GlobalPerspectiveMatrix);
    
	ogl_init_quad_buffers();
	ogl_init_shaders(win32State);
	
	ReleaseDC(window, windowDC);
}

static void draw_quads(QuadBuffer *buffer)
{	
	QuadBuffer quad_buffer = *buffer;	
	glBindTexture(GL_TEXTURE_2D, quad_buffer.texture_handle);    
	    
	//send buffer data to card
	glBindBuffer(GL_ARRAY_BUFFER, GlobalQuadBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Quad) * quad_buffer.quad_count, quad_buffer.quads, GL_STATIC_DRAW);
    
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GlobalQuadIndexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (uint32)(6 * sizeof(uint32) * quad_buffer.quad_count),
                 Global_Index_Buffer, GL_STATIC_DRAW);
    
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 36, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 36, (void*)(12));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 36, (void*)(20));
	glDrawElements(GL_TRIANGLES, 6 * quad_buffer.quad_count, GL_UNSIGNED_INT, (void *)(0));
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

static void win32_ogl_render(HDC device_context, RenderBuffer *buffer)
{
	glClearColor(0.4f, 0.6f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
	//set up camera matrix
	float model_matrix[4][4] = {};	
	float camera_matrix[4][4];
	float MVMatrix[4][4];
	float MVPMatrix[4][4];
	MatrixToIdentity(model_matrix);
	MatrixToIdentity(camera_matrix);
	//todo - figure out how I screwed up the perspective matrix to make the directions backwards?
	MatrixTranslate44(-Global_Camera_Position.x, -Global_Camera_Position.y, -5.0f, camera_matrix);
    
	MatrixMul44(model_matrix, camera_matrix, MVMatrix);
	MatrixMul44(MVMatrix, GlobalPerspectiveMatrix, MVPMatrix);
    
	//shader stuff
	glUseProgram(Global_tex_shader.program_id);	
	glUniformMatrix4fv(Global_tex_shader.MVP_location, 1, false, (GLfloat*)MVPMatrix);
    
	for (int i = 0; i < buffer->buffer_count; i++)
	{
		draw_quads(buffer->quadBuffers[i]);
	}
	
	SwapBuffers(device_context);
}

static void add_quad_to_render_buffer(Quad quad, uint32 texture_handle)
{
	int index = -1;
	for (int i = 0; i < global_render_buffer.buffer_count; i++)
	{
		if (global_render_buffer.quadBuffers[i]->texture_handle == texture_handle)
		{
			index = i;
			break;
		}
	}
    
	if (index == -1)
	{
		Assert(global_render_buffer.buffer_count < QUAD_BUFFER_MAX);
		global_render_buffer.quadBuffers[global_render_buffer.buffer_count] 
			= (QuadBuffer *)VirtualAlloc(0, sizeof(QuadBuffer), MEM_COMMIT, PAGE_READWRITE);
		global_render_buffer.quadBuffers[global_render_buffer.buffer_count]->texture_handle = texture_handle;
		index = global_render_buffer.buffer_count;
		global_render_buffer.buffer_count++;
	}
    
	QuadBuffer * quad_buffer = global_render_buffer.quadBuffers[index];
	Assert(quad_buffer->quad_count < QUAD_BUFFER_SIZE);
	quad_buffer->quads[quad_buffer->quad_count] = quad;
	quad_buffer->quad_count += 1;
}

static void reset_quad_buffers(RenderBuffer * buffer)
{
	for (int i = 0; i < buffer->buffer_count; i++)
	{
		VirtualFree(buffer->quadBuffers[i], 0, MEM_RELEASE);
	}
	buffer->buffer_count = 0;
}