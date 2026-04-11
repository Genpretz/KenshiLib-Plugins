#pragma once

#include <kenshi/util/lektor.h>

template<typename T>
void lektor_push_back(lektor<T>& lek, const T& val)
{
    if (lek.count >= lek.maxSize)
    {
        uint32_t newMax = lek.maxSize == 0 ? 4 : lek.maxSize * 2;

        T* newStuff = (T*)Ogre::AllocatedObject<
            Ogre::CategorisedAllocPolicy<Ogre::MEMCATEGORY_GENERAL>
        >::operator new(newMax * sizeof(T));

        if (lek.stuff)
        {
            memcpy(newStuff, lek.stuff, lek.count * sizeof(T));
            Ogre::AllocatedObject<
                Ogre::CategorisedAllocPolicy<Ogre::MEMCATEGORY_GENERAL>
            >::operator delete(lek.stuff);
        }

        lek.stuff = newStuff;
        lek.maxSize = newMax;
    }

    lek.stuff[lek.count++] = val;
}
