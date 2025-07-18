#include <string>
#include <android/log.h>
#include <pthread.h>
#include "sk.h"
#include <iostream>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>
#include <EGL/eglplatform.h>
#include <random>

#define LOG_TAG "当前日志"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR ,LOG_TAG ,__VA_ARGS__)

static bool isLoop = false;
static pthread_t loopID;
void onStart(ANativeActivity *activity) {
    LOGE("Application开始");
}

void onResume(ANativeActivity *activity) {
    LOGE("onResume");
}

void *onSaveInstanceState(ANativeActivity *activity, size_t *outSize) {
    LOGE("onSaveInstanceState");
    return nullptr;
}

void onPause(ANativeActivity *activity) {
    LOGE("onPause");
}

void onStop(ANativeActivity *activity) {
    LOGE("onStop");

}

void onDestroy(ANativeActivity *activity) {
    LOGE("onDestroy");
}

void onWindowFocusChanged(ANativeActivity *activity, int hasFocus) {
    LOGE("onWindowFocusChanged");
}

void onNativeWindowCreated(ANativeActivity *activity, ANativeWindow *window) {
/**********************************************************************************************
 *
 * 方法1：使用ANativeWindow显示图形
*********************************************************************************************/
/*
   //     声明窗口缓冲
    ANativeWindow_Buffer window_buffer = {0};
    //    画布上锁
    ANativeWindow_lock(window, &window_buffer, nullptr);
    //    计算面积
    int mArea = window_buffer.width * window_buffer.height;
    LOGE("屏幕分辨率是:%d*%d", window_buffer.width, window_buffer.height);
    //获取像素地址
    auto *screen = static_cast<unsigned long *>(window_buffer.bits);
    //循环赋值
    for (int i = mArea/100; i < mArea/6; i++) {
        screen[i] = 0x6633ffaa;
    }
    //解锁并提交绘制
    ANativeWindow_unlockAndPost(window);

    LOGE("Native Window Created !");
    EGLDisplay display1= eglGetDisplay(0);
    EGLint major,minor;
    eglInitialize(display1, &major, &minor);
    LOGE("EGL版本是:%d.%d",major,minor);
    */
/**********************************************************************************************
 *
 * 方法2：使用OpenGLES显示图形
*********************************************************************************************/

    //EGL初始化
    EGLint width, height;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);
    EGLConfig config;
    EGLint configNum;
    EGLint i_num = 8;
    EGLint configSpec[] = {
            EGL_RED_SIZE, i_num,
            EGL_GREEN_SIZE, i_num,
            EGL_BLUE_SIZE, i_num,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };
    eglChooseConfig(display, configSpec, &config, 1, &configNum);
    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
    EGLSurface winsurface = eglCreateWindowSurface(display, config, window, 0);

    eglQuerySurface(display, winsurface, EGL_WIDTH, &width);
    eglQuerySurface(display, winsurface, EGL_HEIGHT, &height);
    LOGE("View Port width: %d, height: %d", width,height);

    eglMakeCurrent(display, winsurface, winsurface, context);
    LOGE("EGL Init Success!");

    unsigned int shaderProgram;
    //    glEnable(GL_CULL_FACE);
    //    glDisable(GL_DEPTH_TEST);
    //    EGLshalder准备

    const char *vertexShaderSource = "#version 300 es\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "out vec4 vertexColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                     "vertexColor = vec4(0.5,0,0,1.0);\n"
                                     "}\0";
    const char *fragmentShaderSource = "#version 300 es\n"
                                       "precision mediump float;\n"
                                       "out vec4 FragColor;\n"
                                       "uniform vec4 ourColor;\n"
                                       "void main()\n"
                                       "{\n"
                                       "FragColor = ourColor;\n"
                                       "}\n\0";


    // build and compile our shader program
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);//创建着色器
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);//将着色器源码附加到着色器对象上
    glCompileShader(vertexShader); //编译
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    shaderProgram  = glCreateProgram(); //创建 program
    glAttachShader(shaderProgram, vertexShader); //着色器与程序附着
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);//链接程序
    glDeleteShader(vertexShader); //释放不需要的shader
    glDeleteShader(fragmentShader);

    //EGL画图
    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
            0.5f, 0.5f, 0.0f,   // 右上角
            0.5f, -0.5f, 0.0f,  // 右下角
            -0.5f, -0.5f, 0.0f, // 左下角
            -0.5f, 0.5f, 0.0f   // 左上角
    };
    unsigned int indices[] = {
            0,2,1
    };
    //VAO:所有顶点数据的状态集合,VAO负责管理顶点属性，而这些顶点属性从0到GL_MAX_VERTEX_ATTRIBS - 1被编号。
    unsigned int VAO,VBO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1,&EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);////绑定VBO 将我们定义的顶点数据填充到VBO中
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);//解除绑定
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glClearColor(0.5f,0.5f,0.5f, 1.0f);//背景色
    glClear(GL_COLOR_BUFFER_BIT);
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    glUseProgram(shaderProgram);
    glUniform4f(vertexColorLocation, 0.1f, 0.8f, 0.9f, 1.0f);//图形颜色
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    eglSwapBuffers(display, winsurface);


}

void onNativeWindowDestroyed(ANativeActivity *activity, ANativeWindow *window) {
    LOGE("onNativeWindowDestroyed");
}

void onInputQueueCreated(ANativeActivity *activity, AInputQueue *queue) {
    LOGE("onInputQueueCreated");
    isLoop = true;
    activity->instance = (void *) queue;
    pthread_create(&loopID, nullptr, looper, activity);
}

void onInputQueueDestroyed(ANativeActivity *activity, AInputQueue *queue) {
    LOGE("onInputQueueDestroyed");
    isLoop = false;
}

void onConfigurationChanged(ANativeActivity *activity) {
    LOGE("onConfigurationChanged");
}

void onLowMemory(ANativeActivity *activity) {
    LOGE("onLowMemory");
}

void bindLifeCycle(ANativeActivity *activity) {
    LOGE("绑定Activity生命周期");
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
}

void *looper(void *args) {
    LOGE("looper start !");
    ANativeActivity *activity = (ANativeActivity *) args;
    AInputQueue *queue = (AInputQueue *) activity->instance;
    AInputEvent *event = nullptr;
    while (isLoop) {
        if (!AInputQueue_hasEvents(queue)) {
            continue;
        }
        AInputQueue_getEvent(queue, &event);
        float mx = AMotionEvent_getX(event, 0);
        float my = AMotionEvent_getY(event, 0);
        switch (AInputEvent_getType(event)) {
            case AINPUT_EVENT_TYPE_MOTION: {
                switch (AMotionEvent_getAction(event)) {
                    case AMOTION_EVENT_ACTION_DOWN: {
//                        LOGE("Touch Screen Down");
                        LOGE("点击的是:%d", int32_t(mx));
                        LOGE("点击的是:%d", int32_t(my));
                        break;
                    }
                    case AMOTION_EVENT_ACTION_UP: {
                        LOGE("手指抬起触发事件");
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case AINPUT_EVENT_TYPE_KEY: {
                switch (AKeyEvent_getAction(event)) {
                    case AKEY_EVENT_ACTION_DOWN: {
                        LOGE("key down");
                        switch (AKeyEvent_getKeyCode(event)) {
                            case AKEYCODE_BACK: {
                                LOGE("BACK down");
                                ANativeActivity_finish(activity);
                                break;
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    case AKEY_EVENT_ACTION_UP: {
                        LOGE("key up");
                        switch (AKeyEvent_getKeyCode(event)) {
                            case AKEYCODE_BACK: {
                                LOGE("BACK up");
                                ANativeActivity_finish(activity);
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            default:
                break;
        }
        AInputQueue_finishEvent(queue, event, 1);
    }
    return args;
}

void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState, size_t savedStateSize) {
    LOGE("初始化Activity");
    bindLifeCycle(activity);
}