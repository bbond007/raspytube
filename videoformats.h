#define AFORMAT_HEIGHT 24
#define AFORMAT_WIDTH 8
char * supported_formats[AFORMAT_HEIGHT][AFORMAT_WIDTH] =
{
    {"Number", "Container", "Resolution", "Video Encoding", "Video Provfile", "Video Bitrate", "Audio Encoding", "Audio Bitrate"},
    {"Auto", "...","...", "...", "...", "...",   "...", "..."},
    {"5", "FLV",    "240p",  "Sorenson H.263",   "N/A",      "0.25",  "MP3",    "64" },
    {"6", "FLV",    "270p",  "Sorenson H.263",   "N/A",      "0.8",   "MP3",    "64" },
    {"13", "3GP",   "N/A",   "MPEG-4 Visual",    "N/A",      "0.5",   "AAC",    "N/A"},
    {"17", "3GP",   "144p",  "MPEG-4 Visual",    "Simple",   "0.05",  "AAC",    "24" },
    {"18", "MP4",   "360p",  "H.264",            "Baseline", "0.5",   "AAC",    "96" },
    {"22", "MP4",   "720p",  "H.264",            "High",     "2-2.9", "AAC",    "192"},
    {"34", "FLV",   "360p",  "H.264",            "Main",     "0.5",   "AAC",    "128"},
    {"35", "FLV",   "480p",  "H.264",            "Main",     "0.8-1", "AAC",    "128"},
    {"36", "3GP",   "240p",  "MPEG-4 Visual",    "Simple",   "0.17",  "AAC",    "38" },
    {"37", "MP4",   "1080p", "H.264",            "High",     "3-4.3", "AAC",    "192"},
    {"38", "MP4",   "3072p", "H.264",            "High",     "3.5-5", "AAC",    "192"},
    {"43", "WebM",  "360p",  "VP8",              "N/A",      "0.5",   "Vorbis", "128"},
    {"44", "WebM",  "480p",  "VP8",              "N/A",      "1",     "Vorbis", "128"},
    {"45", "WebM",  "720p",  "VP8",              "N/A",      "2",     "Vorbis", "192"},
    {"46", "WebM",  "1080p", "VP8",              "N/A",      "N/A",   "Vorbis", "192"},
    {"82", "MP4",   "360p",  "H.264",            "3D",       "0.5",    "AAC",   "96" },
    {"83", "MP4",   "240p",  "H.264",            "3D",       "0.5",   "AAC",    "96" },
    {"84", "MP4",   "720p",  "H.264",            "3D",       "2-2.9", "AAC",    "152"},
    {"85", "MP4",   "520p",  "H.264",            "3D",       "2-2.9", "AAC",    "152"},
    {"100", "WebM", "360p",  "VP8",              "3D",       "N/A",   "Vorbis", "128"},
    {"101", "WebM", "360p",  "VP8",              "3D",       "N/A",   "Vorbis", "192"},
    {"102", "WebM", "720p",  "VP8",              "3D",       "N/A",   "Vorbis", "192"}
};

