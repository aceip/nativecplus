#include <string>
#include <android/log.h>
#include <pthread.h>
#include "native-lib.h"
#include <iostream>
#include <GLES3/gl3.h>

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
    LOGE("Native Window Created !");

    /*声明窗口缓冲*/
    ANativeWindow_Buffer window_buffer = {0};
    /*画布上锁*/
    ANativeWindow_lock(window, &window_buffer, nullptr);
    /*计算面积*/
    int mArea = window_buffer.width * window_buffer.height;
    LOGE("屏幕分辨率是:%d*%d", window_buffer.width, window_buffer.height);
    /*获取像素地址*/
    auto *screen = static_cast<unsigned long *>(window_buffer.bits);
    /*循环赋值*/
    for (int i = mArea / 5; i < mArea / 3; ++i) {
        screen[i] = 0xdd33ffaa;
    }
    /*解锁并提交绘制*/
    ANativeWindow_unlockAndPost(window);

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
/*
 * glActiveTexture (GLenum texture);设置激活的纹理单元(texture unit)
 * glAttachShader (GLuint program, GLuint shader);将shader容器添加到程序中。
 *                        program：着色器程序容器的id。
 *                        shader：要添加的顶点或者片元shader容器的id。
 * glBindAttribLocation (GLuint program, GLuint index, const GLchar *name);把“顶点属性索引”绑定到“顶点属性名”。
 *                              program：着色器程序容器的id。
 *                              index：顶点属性索引。
 *                              name：顶点属性名。
 * glBindBuffer (GLenum target, GLuint buffer);使用生成过的缓冲区
 *                      target：指定缓冲区的目标类型，可以取
 *                      缓冲区目标类型                 用途
                        GL_ARRAY_BUFFER             Vertex attributes 说明该缓冲区存放顶点属性（位置、颜色、纹理等）
                        GL_ATOMIC_COUNTER_BUFFER    Atomic counter storage
                        GL_COPY_READ_BUFFER         Buffer copy source
                        GL_COPY_WRITE_BUFFER        Buffer copy destination
                        GL_DISPATCH_INDIRECT_BUFFER     Indirect compute dispatch commands
                        GL_DRAW_INDIRECT_BUFFER     Indirect command arguments
                        GL_ELEMENT_ARRAY_BUFFER     Vertex array indices 说明该缓冲区存放顶点索引数组
                        GL_PIXEL_PACK_BUFFER        Pixel read target 像素压包操作缓冲区，所谓的像素压包操作就是对像素重新进行格式化，比如以前像素是按照RGB每个颜色分量8个位存储的，压包后可以是按照BRG每个颜色分量2 6 8位的方式存储
                        GL_PIXEL_UNPACK_BUFFER      Texture data source 像素解包操作缓冲区，与压包操作相反
                        GL_QUERY_BUFFER             Query result buffer
                        GL_SHADER_STORAGE_BUFFER    Read-write storage for shaders
                        GL_TEXTURE_BUFFER           Texture data buffer
                        GL_TRANSFORM_FEEDBACK_BUFFER     Transform feedback buffer
                        GL_UNIFORM_BUFFER           Uniform block storage
 * glBindFramebuffer (GLenum target, GLuint framebuffer);使用帧缓冲区
 *                           target:指定帧缓冲区的使用操作类型，可以取GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER or GL_FRAMEBUFFER之一。GL_DRAW_FRAMEBUFFER对帧缓冲区进行写操作（渲染）（这种情况就无法读取帧缓冲区中的颜色、深度等信息，经测试仍能通过glReadPixels读取颜色和深度信息！！！！），GL_READ_FRAMEBUFFER 对帧缓冲区进行读操作，GL_FRAMEBUFFER可以对帧缓冲区进行读写操作。
 *                           framebuffer:创建帧缓冲区时的标识
 * glBindRenderbuffer (GLenum target, GLuint renderbuffer);绑定一个命名的渲染缓冲区对象
 *                            target:指定renderbuffer对象绑定到的目标。 符号常量必须为GL_RENDERBUFFER
 *                            renderbuffer:指定渲染缓冲区对象的名称。
 * glBindTexture (GLenum target, GLuint texture);实现了将调用glGenTextures函数生成的纹理的名字绑定到对应的目标纹理上：函数说明：允许建立一个绑定到目标纹理的有名称的纹理。例如，一幅具有真实感的图像或者照片作为纹理贴到一个矩形上，就可以在定义纹理对象生成纹理对象数组后，通过使用glBindTexture选择纹理对象，来完成该纹理对象的定义。
 *                       target：纹理被绑定的目标，它只能取值GL_TEXTURE_1D或者GL_TEXTURE_2D；
 *                       texture ：纹理的名称，且该纹理的名称在当前的应用中不能被再次使用。
 * glBlendColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
 * glBlendEquation (GLenum mode);设置混合的运算符
 *                  GL_FUNC_ADD：默认选项，将两个分量相加：C¯result=Src+Dst。
                    GL_FUNC_SUBTRACT：将两个分量相减： C¯result=Src−Dst。
                    GL_FUNC_REVERSE_SUBTRACT：将两个分量相减，但顺序相反：C¯result=Dst−Src。
 *  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);对四通道统一进行混合操作
 *              GL_ZERO	因子等于0
                GL_ONE	因子等于1
                GL_SRC_COLOR	因子等于源颜色向量C¯source
                GL_ONE_MINUS_SRC_COLOR	因子等于1−C¯source
                GL_DST_COLOR	因子等于目标颜色向量C¯destination
                GL_ONE_MINUS_DST_COLOR	因子等于1−C¯destination
                GL_SRC_ALPHA	因子等于C¯source的alpha分量
                GL_ONE_MINUS_SRC_ALPHA	因子等于1− C¯source的alpha分量
                GL_DST_ALPHA	因子等于C¯destination的alpha分量
                GL_ONE_MINUS_DST_ALPHA	因子等于1− C¯destination的alpha分量
                GL_CONSTANT_COLOR	因子等于常数颜色向量C¯constant
                GL_ONE_MINUS_CONSTANT_COLOR	因子等于1−C¯constant
                GL_CONSTANT_ALPHA	因子等于C¯constant的alpha分量
                GL_ONE_MINUS_CONSTANT_ALPHA
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);为RGB和alpha通道分别设置不同的混合选项
    glEnable( GL_BLEND ); // 开启混合
    glDisable( GL_BLEND ); // 关闭混合
    glBufferData 和 glNamedBufferData 都是给缓存对象创建新的数据.glBufferData 的缓存对象是当前绑定到 target 中使用的。而 glNamedBufferData的话，缓存对象是个一个buffer的ID。
 *  glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage); 为当前绑定到target的缓冲区对象创建一个新的数据存储，删除任何预先存在的数据存储。
                         target指定目标缓冲区对象。 符号常量必须为GL_ARRAY_BUFFER或GL_ELEMENT_ARRAY_BUFFER。
                         size指定缓冲区对象的新数据存储的大小（以字节为单位）。
                         data指定将复制到数据存储区以进行初始化的数据的指针，如果不复制数据，则指定NULL。
                         usage指定数据存储的预期使用模式。 符号常量必须为GL_STREAM_DRAW，GL_STATIC_DRAW或GL_DYNAMIC_DRAW。
 *
 *
 * */