/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
 *
 *  This file is part of inspectrum.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QMutexLocker>
#include <string.h>
#include "samplebuffer.h"

template <typename Tin, typename Tout>
SampleBuffer<Tin, Tout>::SampleBuffer(std::shared_ptr<SampleSource<Tin>> src) : src(src)
{
	src->subscribe(this);
}

template <typename Tin, typename Tout>
SampleBuffer<Tin, Tout>::~SampleBuffer()
{
	src->unsubscribe(this);
}

template <typename Tin, typename Tout>
std::unique_ptr<Tout[]> SampleBuffer<Tin, Tout>::getSamples(off_t start, off_t length)
{
    // TODO: base this on the actual history required
    auto history = std::min(start, (off_t)256);
    auto samples = src->getSamples(start - history, length + history);
    if (samples == nullptr)
    	return nullptr;

    std::unique_ptr<Tout[]> temp(new Tout[history + length]);
    std::unique_ptr<Tout[]> dest(new Tout[length]);
    QMutexLocker ml(&mutex);
    work(samples.get(), temp.get(), history + length, start);
    memcpy(dest.get(), temp.get() + history, length * sizeof(Tout));
    return dest;
}

template <typename Tin, typename Tout>
void SampleBuffer<Tin, Tout>::invalidateEvent()
{
	SampleSource<Tout>::invalidate();
}

template class SampleBuffer<std::complex<float>, std::complex<float>>;
template class SampleBuffer<std::complex<float>, float>;
template class SampleBuffer<float, float>;
