// -*- Mode: c++ -*-

#ifndef VIDEOOUTBASE_H_
#define VIDEOOUTBASE_H_

// Qt
#include <QSize>
#include <QRect>
#include <QString>
#include <QPoint>
#include <QMap>
#include <qwindowdefs.h>

// MythTV
#include "mythframe.h"
#include "videobuffers.h"
#include "mythcodecid.h"
#include "videoouttypes.h"
#include "videooutwindow.h"
#include "mythdisplay.h"
#include "DisplayRes.h"
#include "videodisplayprofile.h"
#include "videocolourspace.h"
#include "visualisations/videovisual.h"
#include "mythavutil.h"
#include "mythdeinterlacer.h"

using namespace std;

class MythPlayer;
class OSD;
class AudioPlayer;
class MythRender;

typedef QMap<MythPlayer*,PIPLocation> PIPMap;

class MythMultiLocker;

class VideoOutput
{
  public:
    static void GetRenderOptions(render_opts &Options);
    static VideoOutput *Create(const QString &Decoder,    MythCodecID CodecID,
                               PIPState PiPState,         const QSize &VideoDim,
                               const QSize &VideoDispDim, float VideoAspect,
                               QWidget *ParentWidget,     const QRect &EmbedRect,
                               float FrameRate,           uint  PlayerFlags,
                               QString &Codec,            int ReferenceFrames);

    VideoOutput();
    virtual ~VideoOutput();

    virtual bool Init(const QSize &VideoDim, const QSize &VideoDispDim,
                      float VideoAspect,     WId WinID,
                      const QRect &WindowRect, MythCodecID CodecID);
    virtual void InitOSD(OSD *Osd);
    virtual void SetVideoFrameRate(float);
    virtual void SetDeinterlacing(bool Enable, bool DoubleRate);
    virtual void ProcessFrame(VideoFrame *Frame, OSD *Osd, const PIPMap &PipPlayers,
                              FrameScanType Scan = kScan_Ignore) = 0;
    virtual void PrepareFrame(VideoFrame *Frame, FrameScanType, OSD *Osd) = 0;
    virtual void Show(FrameScanType) = 0;
    void         SetReferenceFrames(int ReferenceFrames);
    VideoDisplayProfile *GetProfile() { return m_dbDisplayProfile; }
    virtual void WindowResized(const QSize &) {}
    virtual bool InputChanged(const QSize &VideoDim, const QSize &VideoDispDim,
                              float VideoAspect, MythCodecID  CodecID,
                              bool &AspectChanged, MythMultiLocker* Locks,
                              int   ReferenceFrames);
    virtual void VideoAspectRatioChanged(float VideoAspect);
    virtual void ResizeDisplayWindow(const QRect& Rect, bool SaveVisible);
    virtual void EmbedInWidget(const QRect &EmbedRect);
    bool         IsEmbedding(void);
    virtual void StopEmbedding(void);
    virtual void ResizeForGui(void);
    virtual void ResizeForVideo(int Width = 0, int Height = 0);
    virtual void Zoom(ZoomDirection Direction);
    virtual void ToggleMoveBottomLine(void);
    virtual void SaveBottomLine(void);
    virtual void GetOSDBounds(QRect &Total, QRect &Visible,
                              float &VisibleAspect, float &FontScaling,
                              float ThemeAspect) const;
    QRect        GetMHEGBounds(void);
    virtual float GetDisplayAspect(void) const;
    AspectOverrideMode GetAspectOverride(void) const;
    virtual void ToggleAspectOverride(AspectOverrideMode AspectMode = kAspect_Toggle);
    AdjustFillMode GetAdjustFill(void) const;
    virtual void ToggleAdjustFill(AdjustFillMode FillMode = kAdjustFill_Toggle);
    QString      GetZoomString(void) const;
    PictureAttributeSupported GetSupportedPictureAttributes(void);
    int          ChangePictureAttribute(PictureAttribute AttributeType, bool Direction);
    virtual int  SetPictureAttribute(PictureAttribute Attribute, int NewValue);
    int          GetPictureAttribute(PictureAttribute AttributeType);
    virtual void InitPictureAttributes(void) { }
    virtual bool IsPIPSupported(void) const { return false; }
    virtual bool IsPBPSupported(void) const { return false; }
    virtual bool hasHWAcceleration(void) const { return false; }
    virtual void* GetDecoderContext(unsigned char*, uint8_t*&) { return nullptr; }
    virtual void SetFramesPlayed(long long FramesPlayed);
    virtual long long GetFramesPlayed(void);
    bool         IsErrored() const;
    VideoErrorState GetError(void) const;

    void         SetPrebuffering(bool Normal);
    virtual void ClearAfterSeek(void);
    virtual int  ValidVideoFrames(void) const;
    int          FreeVideoFrames(void);
    bool         EnoughFreeFrames(void);
    bool         EnoughDecodedFrames(void);
    bool         EnoughPrebufferedFrames(void);
    virtual VideoFrameType* DirectRenderFormats(void);
    bool         ReAllocateFrame(VideoFrame *Frame, VideoFrameType Type);
    virtual VideoFrame *GetNextFreeFrame(void);
    virtual void ReleaseFrame(VideoFrame *Frame);
    virtual void DeLimboFrame(VideoFrame *Frame);
    virtual void StartDisplayingFrame(void);
    virtual void DoneDisplayingFrame(VideoFrame *Frame);
    virtual void DiscardFrame(VideoFrame *frame);
    virtual void DiscardFrames(bool KeyFrame);
    virtual void CheckFrameStates(void) { }
    virtual VideoFrame *GetLastDecodedFrame(void);
    virtual VideoFrame *GetLastShownFrame(void);
    QString      GetFrameStatus(void) const;
    virtual void UpdatePauseFrame(int64_t &DisplayTimecode) = 0;

    void         SetVideoResize(const QRect &VideoRect);
    void         SetVideoScalingAllowed(bool Allow);
    virtual void SetVideoFlip(void) { }
    virtual QRect GetPIPRect(PIPLocation Location,
                             MythPlayer *PiPPlayer = nullptr,
                             bool DoPixelAdj = true) const;
    virtual void RemovePIP(MythPlayer *) { }
    virtual void SetPIPState(PIPState Setting);
    virtual QString GetOSDRenderer(void) const;
    virtual MythPainter *GetOSDPainter(void) { return nullptr; }

    QString      GetFilters(void) const;
    QRect        GetImageRect(const QRect &Rect, QRect *DisplayRect = nullptr);
    QRect        GetSafeRect(void);

    bool         EnableVisualisation(AudioPlayer *Audio, bool Enable,
                                     const QString &Name = QString(""));
    virtual bool CanVisualise(AudioPlayer *Audio, MythRender *Render);
    virtual bool SetupVisualisation(AudioPlayer *Audio, MythRender *Render,
                                    const QString &Name);
    VideoVisual* GetVisualisation(void) { return m_visual; }
    QString      GetVisualiserName(void);
    virtual QStringList GetVisualiserList(void);
    void         DestroyVisualisation(void);

    static int   CalcHueBase(const QString &AdaptorName);
    virtual bool StereoscopicModesAllowed(void) const { return false; }
    void SetStereoscopicMode(StereoscopicMode mode) { m_stereo = mode; }
    StereoscopicMode GetStereoscopicMode(void) const { return m_stereo; }

  protected:
    virtual void MoveResize(void);
    virtual void MoveResizeWindow(QRect NewRect) = 0;
    void         InitDisplayMeasurements(int Width, int Height, bool Resize);
    virtual void ShowPIPs(VideoFrame *Frame, const PIPMap &PiPPlayers);
    virtual void ShowPIP(VideoFrame*, MythPlayer*, PIPLocation) { }

    QRect        GetVisibleOSDBounds(float& VisibleAspect,
                                     float& FontScaling,
                                     float ThemeAspect) const;
    QRect        GetTotalOSDBounds(void) const;

    static void  CopyFrame(VideoFrame* To, const VideoFrame* From);

    VideoOutWindow       m_window;
    QSize                m_dbDisplayDimensionsMM;
    VideoColourSpace     m_videoColourSpace;
    AspectOverrideMode   m_dbAspectOverride;
    AdjustFillMode       m_dbAdjustFill;
    LetterBoxColour      m_dbLetterboxColour;
    MythCodecID          m_videoCodecID;
    int                  m_maxReferenceFrames;
    VideoDisplayProfile *m_dbDisplayProfile;
    VideoBuffers         m_videoBuffers;
    VideoErrorState      m_errorState;
    long long            m_framesPlayed;
    DisplayRes          *m_displayRes;
    QSize                m_monitorSize;
    QSize                m_monitorDimensions;
    VideoVisual         *m_visual;
    StereoscopicMode     m_stereo;
    MythAVCopy           m_copyFrame;
    MythDeinterlacer     m_deinterlacer;
};

#endif
