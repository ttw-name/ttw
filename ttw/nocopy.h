#ifndef _TTW_NOCOPY_H
#define _TTW_NOCOPY_H

namespace ttw {
    class NoCopy{
    public:
        NoCopy() =default;
        ~NoCopy() = default;
        NoCopy(NoCopy&&) = delete;
        NoCopy& operator = (NoCopy&&) = delete;
        NoCopy(const NoCopy&) = delete;
        NoCopy& operator = (const NoCopy&) = delete;
    };
}

#endif