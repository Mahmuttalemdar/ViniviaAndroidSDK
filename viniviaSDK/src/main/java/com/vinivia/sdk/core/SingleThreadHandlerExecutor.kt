package com.vinivia.sdk.core

import android.os.Handler
import android.os.HandlerThread
import java.util.concurrent.Executor
import java.util.concurrent.RejectedExecutionException


class SingleThreadHandlerExecutor(private val mThreadName: String, priority: Int) :
    Executor {
    private val mHandlerThread: HandlerThread
    val handler: Handler

    init {
        mHandlerThread = HandlerThread(mThreadName, priority)
        mHandlerThread.start()
        handler = Handler(mHandlerThread.looper)
    }

    override fun execute(command: Runnable) {
        if (!handler.post(command)) {
            throw RejectedExecutionException("$mThreadName is shutting down.")
        }
    }

    fun shutdown(): Boolean {
        return mHandlerThread.quitSafely()
    }
}
