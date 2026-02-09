#ifndef BLESSED_UUID_HPP_
#define BLESSED_UUID_HPP_

namespace blessed
{
    class uuid
    {
    public:
        uuid() {}

        operator bool() const
        {
            return false;
        }

    private:
    };
}

#endif
