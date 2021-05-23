#pragma once

#include <deque>
#include <memory>
#include <mutex>

#include <uv.h>

#include <libvlc_wrapper/vlc_vmem.h>

///////////////////////////////////////////////////////////////////////////////
class VlcVideoOutput :
    private vlc::basic_vmem_wrapper
{
protected:
    VlcVideoOutput();
    ~VlcVideoOutput();

    using vlc::basic_vmem_wrapper::open;
    using vlc::basic_vmem_wrapper::close;

    enum class PixelFormat
    {
        RV32 = 0,
        I420,
    };

    PixelFormat pixelFormat() const
        { return _pixelFormat; }
    void setPixelFormat(PixelFormat format)
        { _pixelFormat = format; }

    class VideoFrame;
    class RV32VideoFrame;
    class I420VideoFrame;

    //should return pointer to buffer for video frame
    virtual void* onFrameSetup(const RV32VideoFrame&) = 0;
    virtual void* onFrameSetup(const I420VideoFrame&) = 0;
    virtual void onFrameReady() = 0;
    virtual void onFrameCleanup() = 0;

    //will reset current flag state
    bool isFrameReady();

private:
    struct VideoEvent;
    struct RV32FrameSetupEvent;
    struct I420FrameSetupEvent;
    struct FrameReadyEvent;
    struct FrameCleanupEvent;

    void handleAsync();

private:
    unsigned video_format_cb(
        char* chroma,
        unsigned* width, unsigned* height,
        unsigned* pitches, unsigned* lines) override;
    void video_cleanup_cb() override;

    void* video_lock_cb(void** planes) override;
    void video_unlock_cb(void* picture, void *const * planes) override;
    void video_display_cb(void* picture) override;

    void notifyFrameReady();

private:
    PixelFormat _pixelFormat; //FIXME! maybe we need std::atomic here
    std::shared_ptr<VideoFrame> _videoFrame; //should be accessed only from decode thread
    std::shared_ptr<VideoFrame> _currentVideoFrame; //should be accessed only from gui thread

    uv_async_t _async;
    std::mutex _guard;
    std::deque<std::unique_ptr<VideoEvent> > _videoEvents;

    std::atomic_flag _waitingFrame;
};

///////////////////////////////////////////////////////////////////////////////
class VlcVideoOutput::VideoFrame
{
protected:
    VideoFrame();
    virtual ~VideoFrame();

public:
    unsigned width() const
        { return _width; }
    unsigned height() const
        { return _height; }
    unsigned size() const
        { return _size; }

    void setFrameBuffer(void* frameBuffer);

protected:
    void* frameBuffer();
    bool frameReady() const;

    virtual unsigned video_format_cb(
        char* chroma,
        unsigned* width, unsigned* height,
        unsigned* pitches, unsigned* lines) = 0;

    virtual void* video_lock_cb(void** planes) = 0;
    virtual void video_unlock_cb(void* picture, void *const * planes);
    void video_cleanup_cb();

    virtual void fillBlack() = 0;

    friend VlcVideoOutput;

protected:
    unsigned _width;
    unsigned _height;
    unsigned _size;

    void* _tmpFrameBuffer;
    std::mutex _guard;
    void* _frameBuffer;
};

///////////////////////////////////////////////////////////////////////////////
class VlcVideoOutput::RV32VideoFrame : public VideoFrame
{
public:
    void fillBlack() override;

private:
    unsigned video_format_cb(
        char* chroma,
        unsigned* width, unsigned* height,
        unsigned* pitches, unsigned* line) override;
    void* video_lock_cb(void** planes) override;
};

///////////////////////////////////////////////////////////////////////////////
class VlcVideoOutput::I420VideoFrame : public VideoFrame
{
public:
    I420VideoFrame();

    unsigned uPlaneOffset() const
        { return _uPlaneOffset; }
    unsigned vPlaneOffset() const
        { return _vPlaneOffset; }

    void fillBlack() override;

private:
    unsigned video_format_cb(
        char* chroma,
        unsigned* width, unsigned* height,
        unsigned* pitches, unsigned* lines) override;
    void* video_lock_cb(void** planes) override;
    void video_unlock_cb(void* picture, void *const * planes) override;

private:
    unsigned _uPlaneOffset;
    unsigned _vPlaneOffset;
};
