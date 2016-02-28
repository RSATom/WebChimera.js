#pragma once

#include <deque>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

#include <uv.h>

#include <libvlc_wrapper\vlc_basic_player.h>

#include <vmem2.h>

///////////////////////////////////////////////////////////////////////////////
class VlcVideoOutput
{
protected:
    VlcVideoOutput();
    ~VlcVideoOutput();

    enum class PixelFormat
    {
        RV32 = 0,
        I420,
    };

    bool open( vlc::basic_player* player );
    void close();

    PixelFormat pixelFormat() const
        { return _pixelFormat; }
    void setPixelFormat( PixelFormat format )
        { _pixelFormat = format; }

    class VideoFrame;
    class RV32VideoFrame;
    class I420VideoFrame;

    //should return pointer to buffer for video frame
    virtual void* onFrameSetup( const RV32VideoFrame& ) = 0;
    virtual void* onFrameSetup( const I420VideoFrame& ) = 0;
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
    static bool vmem2_setup( void* opaque, vmem2_video_format_t* format );
    static bool vmem2_lock( void* opaque, vmem2_planes_t* planes );
    static void vmem2_unlock( void* opaque, const vmem2_planes_t* planes );
    static void vmem2_display( void* opaque, const vmem2_planes_t* planes );
    static void vmem2_cleanup( void* opaque );

    bool setup( vmem2_video_format_t* format );
    bool lock( vmem2_planes_t* planes );
    void unlock( const vmem2_planes_t* planes );
    void display( const vmem2_planes_t* planes );
    void cleanup();

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

    void waitBuffer();
    void setFrameBuffer( void* frameBuffer );

protected:
    virtual bool setup( vmem2_video_format_t* format ) = 0;
    virtual bool lock( vmem2_planes_t* planes ) = 0;

    virtual void fillBlack() = 0;

    friend VlcVideoOutput;

protected:
    unsigned _width;
    unsigned _height;
    unsigned _size;

    std::mutex _guard;
    std::condition_variable _waiter;
    void* _frameBuffer;
};

///////////////////////////////////////////////////////////////////////////////
class VlcVideoOutput::RV32VideoFrame : public VideoFrame
{
public:
    void fillBlack() override;

private:
    bool setup( vmem2_video_format_t* format ) override;
    bool lock( vmem2_planes_t* planes ) override;
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
    bool setup( vmem2_video_format_t* format ) override;
    bool lock( vmem2_planes_t* planes ) override;

private:
    unsigned _uPlaneOffset;
    unsigned _vPlaneOffset;
};
