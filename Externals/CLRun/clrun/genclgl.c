// Automatically generated by generateClRun.pl
#include "dynamiclib.h"
#include "../include/CL/cl_gl.h"


static cl_mem (*clCreateFromGLBuffer_ptr)(cl_context, cl_mem_flags, cl_GLuint, int *) = NULL;
cl_mem CL_API_CALL clCreateFromGLBuffer (cl_context context,cl_mem_flags flags,cl_GLuint bufobj,int * errcode_ret) {
	if(!clCreateFromGLBuffer_ptr) clCreateFromGLBuffer_ptr = getFunction("clCreateFromGLBuffer");
	return (*clCreateFromGLBuffer_ptr)(context, flags, bufobj, errcode_ret);
}

static cl_mem (*clCreateFromGLTexture_ptr)(cl_context, cl_mem_flags, cl_GLenum, cl_GLint, cl_GLuint, cl_int *) = NULL;
cl_mem CL_API_CALL clCreateFromGLTexture (cl_context context,cl_mem_flags flags,cl_GLenum target,cl_GLint miplevel,cl_GLuint texture,cl_int * errcode_ret) {
	if(!clCreateFromGLTexture_ptr) clCreateFromGLTexture_ptr = getFunction("clCreateFromGLTexture");
	return (*clCreateFromGLTexture_ptr)(context, flags, target, miplevel, texture, errcode_ret);
}

static cl_mem (*clCreateFromGLRenderbuffer_ptr)(cl_context, cl_mem_flags, cl_GLuint, cl_int *) = NULL;
cl_mem CL_API_CALL clCreateFromGLRenderbuffer (cl_context context,cl_mem_flags flags,cl_GLuint renderbuffer,cl_int * errcode_ret) {
	if(!clCreateFromGLRenderbuffer_ptr) clCreateFromGLRenderbuffer_ptr = getFunction("clCreateFromGLRenderbuffer");
	return (*clCreateFromGLRenderbuffer_ptr)(context, flags, renderbuffer, errcode_ret);
}

static cl_int (*clGetGLObjectInfo_ptr)(cl_mem, cl_gl_object_type *, cl_GLuint *) = NULL;
cl_int CL_API_CALL clGetGLObjectInfo (cl_mem memobj,cl_gl_object_type * gl_object_type,cl_GLuint * gl_object_name) {
	if(!clGetGLObjectInfo_ptr) clGetGLObjectInfo_ptr = getFunction("clGetGLObjectInfo");
	return (*clGetGLObjectInfo_ptr)(memobj, gl_object_type, gl_object_name);
}

static cl_int (*clGetGLTextureInfo_ptr)(cl_mem, cl_gl_texture_info, size_t, void *, size_t *) = NULL;
cl_int CL_API_CALL clGetGLTextureInfo (cl_mem memobj,cl_gl_texture_info param_name,size_t param_value_size,void * param_value,size_t * param_value_size_ret) {
	if(!clGetGLTextureInfo_ptr) clGetGLTextureInfo_ptr = getFunction("clGetGLTextureInfo");
	return (*clGetGLTextureInfo_ptr)(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

static cl_int (*clEnqueueAcquireGLObjects_ptr)(cl_command_queue, cl_uint, const cl_mem *, cl_uint, const cl_event *, cl_event *) = NULL;
cl_int CL_API_CALL clEnqueueAcquireGLObjects (cl_command_queue command_queue,cl_uint num_objects,const cl_mem * mem_objects,cl_uint num_events_in_wait_list,const cl_event * event_wait_list,cl_event * event) {
	if(!clEnqueueAcquireGLObjects_ptr) clEnqueueAcquireGLObjects_ptr = getFunction("clEnqueueAcquireGLObjects");
	return (*clEnqueueAcquireGLObjects_ptr)(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}

static cl_int (*clEnqueueReleaseGLObjects_ptr)(cl_command_queue, cl_uint, const cl_mem *, cl_uint, const cl_event *, cl_event *) = NULL;
cl_int CL_API_CALL clEnqueueReleaseGLObjects (cl_command_queue command_queue,cl_uint num_objects,const cl_mem * mem_objects,cl_uint num_events_in_wait_list,const cl_event * event_wait_list,cl_event * event) {
	if(!clEnqueueReleaseGLObjects_ptr) clEnqueueReleaseGLObjects_ptr = getFunction("clEnqueueReleaseGLObjects");
	return (*clEnqueueReleaseGLObjects_ptr)(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}

static CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem (*clCreateFromGLTexture2D_ptr)(cl_context, cl_mem_flags, cl_GLenum, cl_GLint, cl_GLuint, cl_int *) = NULL;
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL clCreateFromGLTexture2D (cl_context context,cl_mem_flags flags,cl_GLenum target,cl_GLint miplevel,cl_GLuint texture,cl_int * errcode_ret) {
	if(!clCreateFromGLTexture2D_ptr) clCreateFromGLTexture2D_ptr = getFunction("clCreateFromGLTexture2D");
	return (*clCreateFromGLTexture2D_ptr)(context, flags, target, miplevel, texture, errcode_ret);
}

static CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem (*clCreateFromGLTexture3D_ptr)(cl_context, cl_mem_flags, cl_GLenum, cl_GLint, cl_GLuint, cl_int *) = NULL;
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL clCreateFromGLTexture3D (cl_context context,cl_mem_flags flags,cl_GLenum target,cl_GLint miplevel,cl_GLuint texture,cl_int * errcode_ret) {
	if(!clCreateFromGLTexture3D_ptr) clCreateFromGLTexture3D_ptr = getFunction("clCreateFromGLTexture3D");
	return (*clCreateFromGLTexture3D_ptr)(context, flags, target, miplevel, texture, errcode_ret);
}

static cl_int (*clGetGLContextInfoKHR_ptr)(const cl_context_properties *, cl_gl_context_info, size_t, void *, size_t *) = NULL;
cl_int CL_API_CALL clGetGLContextInfoKHR (const cl_context_properties * properties,cl_gl_context_info param_name,size_t param_value_size,void * param_value,size_t * param_value_size_ret) {
	if(!clGetGLContextInfoKHR_ptr) clGetGLContextInfoKHR_ptr = getFunction("clGetGLContextInfoKHR");
	return (*clGetGLContextInfoKHR_ptr)(properties, param_name, param_value_size, param_value, param_value_size_ret);
}
