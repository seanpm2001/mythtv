#include <algorithm>
using namespace std;

#include "tv.h"
#include "tv_play.h"
#include "tv_rec.h"

/**
 *  \brief Returns a human readable QString representing a TVState.
 *  \param state State to print.
 */
QString StateToString(TVState state)
{
    QString statestr = QString("Unknown(%1)").arg((int)state);
    switch (state) {
        case kState_Error: statestr = "Error"; break;
        case kState_None: statestr = "None"; break;
        case kState_WatchingLiveTV:
            statestr = "WatchingLiveTV";
            break;
        case kState_WatchingPreRecorded:
            statestr = "WatchingPreRecorded";
            break;
        case kState_WatchingVideo:
            statestr = "WatchingVideo";
            break;
        case kState_WatchingDVD:
            statestr = "WatchingDVD";
            break;
        case kState_WatchingBD:
            statestr = "WatchingBD";
            break;
        case kState_WatchingRecording:
            statestr = "WatchingRecording";
            break;
        case kState_RecordingOnly:
            statestr = "RecordingOnly";
            break;
        case kState_ChangingState: statestr = "ChangingState"; break;
    }
    statestr.detach();
    return statestr;
}

/**
 *  \brief Returns a translatable QString representing a TVState.
 *  \param state State to print.
 */
QString StateToPublicString(TVState state)
{
    QString str = QString();
    switch (state) {
        case kState_WatchingLiveTV:
            str = QObject::tr("Live TV");
            break;
        case kState_WatchingPreRecorded:
            str =  QObject::tr("Prerecorded");
            break;
        case kState_WatchingRecording:
            str =  QObject::tr("Recording");
            break;
        case kState_WatchingVideo:
            str =  QObject::tr("Video");
            break;
        case kState_WatchingDVD:
            str =  QObject::tr("DVD");
            break;
        case kState_WatchingBD:
            str =  QObject::tr("Blu-ray");
            break;
        case kState_RecordingOnly:
        case kState_Error:
        case kState_None:
        case kState_ChangingState:
        default:
            break;
    }
    str.detach();
    return str;
}

QString toTypeString(PictureAdjustType type)
{
    const QString kPicAdjType[] =
    {
        "",
        "",
        QObject::tr("(CH)"),
        QObject::tr("(REC)"),
    };

    QString ret = kPicAdjType[(uint)type & 0x3];
    ret.detach();

    return ret;
}

QString toTitleString(PictureAdjustType type)
{
    const QString kPicAdjTitles[] =
    {
        "",
        QObject::tr("Adjust Playback"),
        QObject::tr("Adjust Recorder"),
        QObject::tr("Adjust Recorder"),
    };

    QString ret = kPicAdjTitles[(uint)type & 0x3];
    ret.detach();

    return ret;
}

QString toString(CommSkipMode type)
{
    const QString kCommSkipTitles[] =
    {
        QObject::tr("Auto-Skip OFF"),
        QObject::tr("Auto-Skip ON"),
        QObject::tr("Auto-Skip Notify"),
    };

    QString ret = kCommSkipTitles[(uint)(type) % kCommSkipCount];
    ret.detach();

    return ret;
}
