/*
 * This work was authored by Two Six Labs, LLC and is sponsored by a subcontract
 * agreement with Galois, Inc.  This material is based upon work supported by
 * the Defense Advanced Research Projects Agency (DARPA) under Contract No.
 * HR0011-19-C-0103.
 *
 * The Government has unlimited rights to use, modify, reproduce, release,
 * perform, display, or disclose computer software or computer software
 * documentation marked with this legend. Any reproduction of technical data,
 * computer software, or portions thereof marked with this legend must also
 * reproduce this marking.
 *
 * Copyright 2020 Two Six Labs, LLC.  All rights reserved.
 */

#include <argp.h>

#include <sstream>

#include "options.hpp"

const int OPT_THRESH    = 129;
const int OPT_CODEC     = 131;
const int OPT_OUT_DIR   = 132;
const int OPT_MAX_OUT   = 133;
const int OPT_SLIDE     = 134;
const int OPT_LIMIT     = 135;
const int OPT_INC       = 136;
const int OPT_LOGLEVEL  = 137;
const int OPT_KBD       = 138;
const int OPT_FREESPACE = 139;

static struct argp_option options[] =
{
    { 0,              0,             0,             0, "video options:",                            1 },
    { "video_device", 'd',           "device",      0, "video device",                              0 },
    { "video_type",   't',           "type",        0, "video type (jpeg|yuyv|h264|stream)",        0 },
    { "width",        'W',           "pixels",      0, "image width",                               0 },
    { "height",       'H',           "pixels",      0, "image height",                              0 },
    { "flip",         'f',           "v|h",         0, "horizontal or vertical image flip",         0 },
    { "decoder",      'D',           "url",         0, "MPEG-TS H.264 decoder url (host:port)",     0 },
    { 0,              0,             0,             0, "frame processor options:",                  2 },
    { "color_track",  'C',           "RRGGBB",      0, "color tracking (RGB hex)",                  0 },
    { "threshold",    OPT_THRESH,    "val",         0, "color tracking threshold",                  0 },
    { "xwindows",     'X',           NULL,          0, "xwindows frame processor",                  0 },
    { "filesystem",   'F',           NULL,          0, "filesystem frame processor",                0 },
    { "encoder",      'E',           "url",         0, "MPEG-TS H.264 encoder url (host:port)",     0 },
    { "codec",        OPT_CODEC,     "type",        0, "encoder codec (mpeg1|mpeg2|h264)",          0 },
    { "out_dir",      OPT_OUT_DIR,   "path",        0, "image output directory",                    0 },
    { "out_count",    OPT_MAX_OUT,   "val",         0, "image output maximum file count",           0 },
    { "sliding",      OPT_SLIDE,     NULL,          0, "sliding window image filter",               0 },
    { 0,              0,             0,             0, "input/output options:",                     3 },
    { "in_keyboard",  OPT_KBD,       NULL,          0, "read position input from keyboard",         0 },
    { "in_freespace", OPT_FREESPACE, NULL,          0, "read position input from freespace device", 0 },
    { "output",       'o',           "servo|print", 0, "angular position output",                   0 },
    { "output_limit", OPT_LIMIT,     "val",         0, "angular position bound",                    0 },
    { "output_incr",  OPT_INC,       "val",         0, "angular position increment",                0 },
    { "verbose",      'v',           NULL,          0, "verbose output",                            4 },
    { "loglevel",     OPT_LOGLEVEL,  "val",         0, "ffmpeg libraries log level",                0 },
    { NULL,            0 ,           NULL,          0, NULL,                                        0 },
};

static std::string parseStreamUrl(std::string url, struct argp_state * state, bool encoder)
{
    std::string msg = encoder ? "encoder" : "decoder";
    if (url.find(':') == std::string::npos)
    {
        argp_error(state, "mpeg-ts %s argument '%s' must be host:port", msg.c_str(), url.c_str());
        return "";
    }
    if (url.find("udp://") == 0)
    {
        return url;
    }
    else if (url.find("://") != std::string::npos)
    {
        argp_error(state, "mpeg-ts %s argument '%s' must be host:port", msg.c_str(), url.c_str());
        return "";
    }
    else
    {
        return "udp://" + url;
    }
}

static error_t parseOpt(int key, char * arg, struct argp_state * state)
{
    Options * opt = static_cast<Options *>(state->input);
    std::istringstream ss(arg != NULL ? arg : "");

    switch (key)
    {
        case 'd':
            ss >> opt->mVideoDevice;
            break;

        case 't':
            if (ss.str() == "jpeg")
            {
                opt->mVideoInputType = VIDEO_JPEG;
                opt->mVideoOutputType = VIDEO_JPEG;
            }
            else if (ss.str() == "yuyv")
            {
                opt->mVideoInputType = VIDEO_YUYV;
                opt->mVideoOutputType = VIDEO_YUYV;
            }
            else if (ss.str() == "h264")
            {
                opt->mVideoInputType = VIDEO_H264;
                opt->mVideoOutputType = VIDEO_H264;
            }
            else if (ss.str() == "stream")
            {
                opt->mVideoInputType = VIDEO_STREAM;
                opt->mVideoOutputType = VIDEO_YUYV;
            }
            else
            {
                argp_error(state, "invalid video type argument '%s'", arg);
            }
            break;

        case OPT_CODEC:
            if (ss.str() == "mpeg1")
            {
                opt->mEncoderCodecType = CODEC_MPEG1;
            }
            else if (ss.str() == "mpeg2")
            {
                opt->mEncoderCodecType = CODEC_MPEG2;
            }
            else if (ss.str() == "h264")
            {
                opt->mEncoderCodecType = CODEC_H264;
            }
            else
            {
                argp_error(state, "invalid codec type argument '%s'", arg);
            }
            break;

        case 'W':
            ss >> opt->mImageWidth;
            break;

        case 'H':
            ss >> opt->mImageHeight;
            break;

        case 'f':
            if (ss.str() == "v")
            {
                opt->mImageVerticalFlip = true;
            }
            else if (ss.str() == "h")
            {
                opt->mImageHorizontalFlip = true;
            }
            else
            {
                argp_error(state, "invalid flip argument '%s'", arg);
            }

            break;

        case OPT_OUT_DIR:
            ss >> opt->mImageOutputDirectory;
            break;

        case OPT_MAX_OUT:
            ss >> opt->mImageOutputMaxFiles;
            break;

        case 'o':
            if (ss.str() == "servo")
            {
                opt->mOutputType = PiServo;
            }
            else if (ss.str() == "print")
            {
                opt->mOutputType = Print;
            }
            else
            {
                argp_error(state, "invalid output argument '%s'", arg);
            }
            break;

        case OPT_KBD:
            opt->mInputKeyboard = true;
            break;

        case OPT_FREESPACE:
            opt->mInputFreespace = true;
            break;

        case 'X':
            opt->mXWinProcessor = true;
            break;

        case 'F':
            opt->mFilesystemProcessor = true;
            break;

        case 'E':
            opt->mH264Encoder = true;
            opt->mH264EncoderUrl = parseStreamUrl(ss.str(), state, true);
            break;

        case 'D':
            opt->mH264DecoderUrl = parseStreamUrl(ss.str(), state, false);
            break;

        case OPT_LIMIT: {
            float limit;
            ss >> limit;
            opt->mAngularPositionMin = -limit;
            opt->mAngularPositionMax = limit;
            break;
        }

        case OPT_INC:
            ss >> opt->mAngularPositionIncrement;
            if ((opt->mAngularPositionIncrement <= 0.0) || (opt->mAngularPositionIncrement > 5.0)) {
                argp_error(state, "angular position increment must be between 0.0 and 5.0: '%s'", arg);
            }
            break;

        case OPT_SLIDE:
            opt->mImageSlidingWindow = true;
            break;

        case 'C': {
            std::string argval = ss.str();
            opt->mImageTracking = true;
            if (argval.length() != 6)
            {
                argp_error(state, "invalid length of color tracking argument '%s'", arg);
            }
            for (size_t i = 0; i < 6; i++)
            {
                size_t idx = i / 2;
                size_t shift = ((i + 1) % 2);
                char c = argval[i];
                int val = -1;
                if ((c >= '0') && (c <= '9')) {
                    val = c - '0';
                } else if (c >= 'a' && c <= 'f') {
                    val = c - 'a' + 10;
                } else if (c >= 'A' && c <= 'F') {
                    val = c - 'A' + 10;
                }
                if (val < 0) {
                    argp_error(state, "invalid RRGGBB color tracking argument '%s'", arg);
                } else {
                    opt->mImageTrackingRGB[idx] += (val) << (shift ? 4 : 0);
                }
            }
            break;
        }

        case OPT_THRESH:
            ss >> opt->mImageTrackingThreshold;
            break;

        case 'v':
            opt->mVerbose = true;
            break;

        case OPT_LOGLEVEL:
            ss >> opt->mFFmpegLogLevel;
            break;
    }

    return 0;
}

void parseArgs(int argc, char * argv[], Options * opt)
{
    struct argp argp;
    argp.options = options;
    argp.parser = parseOpt;
    argp.args_doc = "test";
    argp.doc = "Embedded application based on camera, position input and position driver";
    argp.children = NULL;
    argp.help_filter = NULL;
    argp.argp_domain = NULL;

    argp_parse(&argp, argc, argv, 0, 0, opt);
}