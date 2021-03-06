//============================================================================
//
// This file is part of the Thea project.
//
// This software is covered by the following BSD license, except for portions
// derived from other works which are covered by their respective licenses.
// For full licensing information including reproduction of these external
// licenses, see the file LICENSE.txt provided in the documentation.
//
// Copyright (C) 2013, Siddhartha Chaudhuri/Princeton University
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holders nor the names of contributors
// to this software may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//============================================================================

/*
 ORIGINAL HEADER

 @file Random.h

 @maintainer Morgan McGuire, http://graphics.cs.williams.edu

 @created 2009-01-02
 @edited  2009-03-20

 Copyright 2000-2009, Morgan McGuire.
 All rights reserved.
*/

#ifndef __Thea_Random_hpp__
#define __Thea_Random_hpp__

#include "Common.hpp"
#include "Spinlock.hpp"
#include <algorithm>

namespace Thea {

/**
 * Threadsafe random number generator. Useful for generating consistent random numbers across platforms and when multiple
 * threads are involved.
 *
 * Uses the Fast Mersenne Twister (FMT-19937) algorithm.
 *
 * Derived from the G3D library: http://g3d.sourceforge.net
 *
 * @cite SFMT http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index.html
 */
class THEA_API Random
{
  protected:
    /** Constants (important for the algorithm; do not modify) */
    enum
    {
      N = 624,
      M = 397,
      R = 31,
      U = 11,
      S = 7,
      T = 15,
      L = 18,
      A = 0x9908B0DF,
      B = 0x9D2C5680,
      C = 0xEFC60000
    };

    /** Prevents multiple overlapping calls to generate(). */
    Spinlock     lock;

    /** State vector (these are the next N values that will be returned). */
    uint32   *   state;

    /** Index into state. */
    int32        index;

    /** Flag that can be set to false to deactivate locking and improve performance in single-threaded scenarios. */
    bool         m_threadsafe;

    /** Generate the next N ints, and store them for readback later. Called from bits(). */
    virtual void generate();

    /** For subclasses. The void * parameter is just to distinguish this from the public constructor. */
    Random(void *);

    /** Get a random number to seed the generator. */
    static uint32 getRandomSeed();

  public:
    /** The maximum signed integral value that can be generated by this class (similar to RAND_MAX). */
    static int32 const MAX_INTEGER = 0x7FFFFFFF;

    /**
     * Constructor.
     *
     * @param seed Use this to specify your own starting random seed if necessary.
     * @param threadsafe Set to false if you know that this random will only be used on a single thread. This eliminates the
     *   lock and could improve performance.
     */
    Random(uint32 seed = getRandomSeed(), bool threadsafe = true);

    /** Destructor. */
    virtual ~Random();

    /**
     * Get a sequence of random bits. Subclasses can choose to override just this method and the other methods will all work
     * automatically.
     */
    virtual uint32 bits();

    /** Toss a fair coin. */
    virtual bool coinToss();

    /** Uniform random integer in the range [0, MAX_INTEGER] (both endpoints inclusive). Similar to std::rand(). */
    virtual int32 integer();

    /** Uniform random integer in the range [lo, hi] (both endpoints inclusive). Negative arguments are ok. */
    virtual int32 integer(int32 lo, int32 hi);

    /** Uniform random real number in the range [lo, hi]. Negative arguments are ok. */
    virtual Real uniform(Real lo, Real hi)
    {
      // We could compute the ratio in double precision here for about 1.5x slower performance and slightly better precision.
      return lo + (hi - lo) * ((Real)bits() / (Real)0xFFFFFFFFUL);
    }

    /** Uniform random real number in the range [0, 1]. */
    virtual Real uniform01()
    {
      // We could compute the ratio in double precision here for about 1.5x slower performance and slightly better precision.
      static Real const NORM = 1.0f / (Real)0xFFFFFFFFUL;
      return (Real)bits() * NORM;
    }

    /** Generate normally distributed real numbers. */
    virtual Real gaussian(Real mean, Real stddev);

    /** Generate 3D unit vectors distributed according to a cosine distribution about the z-axis. */
    virtual void cosHemi(Real & x, Real & y, Real & z);

    /**
     * Generate 3D unit vectors distributed according to a cosine power distribution (\f$ \cos^k \theta \f$) about the z-axis.
     */
    virtual void cosPowHemi(Real k, Real & x, Real & y, Real & z);

    /** Generate 3D unit vectors uniformly distributed on the hemisphere about the z-axis. */
    virtual void hemi(Real & x, Real & y, Real & z);

    /** Generate 3D unit vectors uniformly distributed on the sphere */
    virtual void sphere(Real & x, Real & y, Real & z);

    /**
     * Get \a m distinct random integers in [\a lo, \a hi] (endpoints inclusive). Slow if the range is very large, and possibly
     * breaks if the upper limit of the range is very large (more than MAX_INTEGER / 2).
     *
     * This function is similar to randomShuffle(int32, int32, T *), but trades speed for reduced memory usage
     * (randomShuffle(int32, int32, T *) requires an array of size \a hi - \a lo + 1 -- integers() requires essentially no extra
     * memory).
     *
     * From http://my.opera.com/subjam/blog/book-review-programming-pearls . This implements Algorithm S in Section 3.4.2 of
     * Knuth's Seminumerical Algorithms.
     *
     * @param lo Lower limit of range (inclusive).
     * @param hi Upper limit of range (inclusive).
     * @param m Number of distinct random integers requested from the range.
     * @param selected Used to return the generated random integers. Assumed to be preallocated to \a m elements.
     */
    virtual void integers(int32 lo, int32 hi, int32 m, int32 * selected);

    /**
     * Get \a m distinct random integers in [\a lo, \a hi] (endpoints inclusive), sorted in ascending order. Slow if the range
     * is very large, and possibly breaks if the upper limit of the range is very large (more than MAX_INTEGER / 2).
     *
     * From http://my.opera.com/subjam/blog/book-review-programming-pearls . This implements Algorithm S in Section 3.4.2 of
     * Knuth's Seminumerical Algorithms.
     *
     * @param lo Lower limit of range (inclusive).
     * @param hi Upper limit of range (inclusive).
     * @param m Number of distinct random integers requested from the range.
     * @param selected Used to return the generated random integers in ascending order. Assumed to be preallocated to \a m
     * elements.
     */
    virtual void sortedIntegers(int32 lo, int32 hi, int32 m, int32 * selected);

    /** Randomly shuffle a set of \a n elements. This implements the Fisher-Yates/Knuth shuffle. */
    template <typename T> void randomShuffle(int32 n, T * elems)
    {
      randomShuffle(n, n, elems);
    }

    /**
     * Get the first \a k elements of a random permutation of a set of \a n elements. This implements a partial
     * Fisher-Yates/Knuth shuffle.
     *
     * @param n The number of elements in the complete set.
     * @param k The size of the desired prefix of the random permutation.
     * @param elems The input set, also used to return the desired prefix of the random permutation. The function will set the
     *   first \a k entries of this array to be the computed prefix. The remaining \a n - \a k entries will contain the
     *   remainder of the elements, but not in random order.
     */
    template <typename T> void randomShuffle(int32 n, int32 k, T * elems)
    {
      for (int32 i = 0; i < k; i++)
      {
        int32 j = (integer() % (n - i)) + i;
        std::swap(elems[i], elems[j]);
      }
    }

    /**
     * A shared, threadsafe random number generator. Suggested for general usage when a separate object is not required (e.g. as
     * a replacement for std::rand()).
     */
    static Random & common()
    {
      static Random r;
      return r;
    }

}; // class Random

} // namespace Thea

#endif
