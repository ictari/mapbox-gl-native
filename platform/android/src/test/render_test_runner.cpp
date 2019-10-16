
#include "jni.hpp"
#include "logger.hpp"
#include <mbgl/render_test.hpp>
#include <android_native_app_glue.h>

#include <string>
#include <vector>

#include <mbgl/util/logging.hpp>

#include <android/log.h>


// Find a class, attempting to load the class if it's not found.
jclass LoadClass(JNIEnv* env, jobject activity_object, const char* class_name) {
    jclass class_object = env->FindClass(class_name);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        // If the class isn't found it's possible NativeActivity is being used by
        // the application which means the class path is set to only load system
        // classes.  The following falls back to loading the class using the
        // Activity before retrieving a reference to it.
//        jclass activity_class = env->FindClass("android/app/NativeActivity");
        jclass activity_class = env->GetObjectClass(activity_object);
        jmethodID activity_get_class_loader = env->GetMethodID(
                activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");

        jobject class_loader_object =
                env->CallObjectMethod(activity_object, activity_get_class_loader);

        jclass class_loader_class = env->FindClass("java/lang/ClassLoader");
        jmethodID class_loader_load_class =
                env->GetMethodID(class_loader_class, "loadClass",
                                 "(Ljava/lang/String;)Ljava/lang/Class;");
        jstring class_name_object = env->NewStringUTF(class_name);

        class_object = static_cast<jclass>(env->CallObjectMethod(
                class_loader_object, class_loader_load_class, class_name_object));

        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            class_object = nullptr;
        }
        env->DeleteLocalRef(class_name_object);
        env->DeleteLocalRef(class_loader_object);
    }
//    return class_object;
}




namespace mbgl {

    namespace {

        int severityToPriority(EventSeverity severity) {
                switch(severity) {
                    case EventSeverity::Debug:
                            return ANDROID_LOG_DEBUG;

                                case EventSeverity::Info:
                            return ANDROID_LOG_INFO;

                                case EventSeverity::Warning:
                            return ANDROID_LOG_WARN;

                               case EventSeverity::Error:
                            return ANDROID_LOG_ERROR;

                                default:
                            return ANDROID_LOG_VERBOSE;
                        }
            }

        } // namespace

    void Log::platformRecord(EventSeverity severity, const std::string &msg) {
         __android_log_print(severityToPriority(severity), "mbgl", "%s", msg.c_str());

    }

}


void android_main(struct android_app* app) {
    using FindClassFN = jclass (*)(JNIEnv*, const char*);
    mbgl::android::theJVM= app->activity->vm;
    JNIEnv* env;
    app->activity->vm->AttachCurrentThread(&env, NULL);
    // Logger

//    LoadClass(env, app->activity->clazz, mbgl::android::Logger::Name());
//    mbgl::android::Logger::registerNative(*env);
     std::vector<std::string> arguments = {"runner","-p", "/sdcard/render-test"};
     std::vector<char*> argv;
     for (const auto& arg : arguments) {
         argv.push_back((char*) arg.data());
     }
     argv.push_back(nullptr);
     mbgl::runRenderTests(argv.size() - 1, argv.data());
    app->activity->vm->DetachCurrentThread();
}