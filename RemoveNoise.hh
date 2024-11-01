/*

    Background noise estimation and removal, header
    Copyright (C) 2000-2001 Jussi Laako

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#include <Alloc.hh>
#include <dsp/DSPOp.hh>


#ifndef REMOVENOISE_HH
    #define REMOVENOISE_HH


    /**
        Background noise estimation and removal
    */
    class clRemoveNoise : private clDSPOp
    {
            long lPrevSize;
            clAlloc PrevBuf;
            void GetPosSize (long *, long *, long, long, long);
            void GetPosSize1 (long *, long *, long, long, long, long);
            void GetPosSize2 (long *, long *, long, long, long, long);
            long GetAlphaMedian (const float *, float, long);
            long GetAlphaMedian (const double *, double, long);
        public:
            clRemoveNoise ();
            ~clRemoveNoise ();
            /**
                Two-Pass Split-Window algorithm

                \f[R_{k}=\{k-M,k-M+1,...,k-L,k+L,...,k+M\}, 0\leq{L}<{M}\f]
                \f[K=\left\{\begin{array}{ll}
                2M+1&, L=0\\
                2M+2-2L&, L\neq{0}
                \end{array}\right.\f]
                \f[\hat{x}(k)=\frac{1}{K}\sum_{i\epsilon{R_{k}}}x(i)\f]
                \f[y(k)=\left\{\begin{array}{ll}
                x(k)&, x(k)\leq\alpha\hat{x}(k)\\
                \hat{x}(k)&, x(k)>\alpha\hat{x}(k)
                \end{array}\right.\f]
                \f[\hat{m}(k)=\frac{1}{K}\sum_{i\epsilon{R_{k}}}y(i)\f]
                then
                \f[z'(k)=\frac{x(k)-\hat{m}(k)}{\hat{m}(k)}, 0\leq{k}\leq{N-1}\f]

                \param fpVect Source&destination vector
                \param fAlpha Alpha
                \param lMeanLength Mean window length
                \param lGapLength Gap length
                \param lLength Length of source&destination vector
            */
            void TPSW (float *, float, long, long, long);
            /// \overload
            void TPSW (double *, double, long, long, long);
            /**
                \overload
                \param fpDest Destination vector
                \param fpSrc Source vector
                \param lMeanLength Mean window length
                \param lGapLength Gap length
                \param lLength Length of source&destination vectors
            */
            void TPSW (float *, const float *, float, long, long, long);
            /// \overload
            void TPSW (double *, const double *, double, long, long, long);
            /**
                Order-Truncate-Average algorithm

                Let y(k) be sorted (ascending) x(k) and \f$Y_{sm}\f$ median, 
                then
                \f[\hat{m}(k)=\frac{1}{I}\sum_{\stackrel{i=1}{i\epsilon{R_{k}}}}^{I}y(i), 
                y(I)\leq\alpha{Y_{sm}}, 
                y(I+1)>\alpha{Y_{sm}}\f]
                then
                \f[z'(k)=\frac{x(k)-\hat{m}(k)}{\hat{m}(k)}, 0\leq{k}\leq{N-1}\f]

                \param fpVect Source&destination vector
                \param fAlpha Alpha
                \param lMeanLength Mean length
                \param lLength Length of source&destination vector
            */
            void OTA (float *, float, long, long);
            /// \overload
            void OTA (double *, double, long, long);
            /**
                \param fpDest Destination vector
                \param fpSrc Source vector
                \param fAlpha Alpha
                \param lMeanLength Mean length
                \param lLength Length of source&destination vectors
            */
            void OTA (float *, const float *, float, long, long);
            /// \overload
            void OTA (double *, const double *, double, long, long);
            /**
                Differential method (experimental)

                \f[z'(k)=x_{1}(k)w-x_{2}(k)(1-w), 0\leq{k}\leq{N-1}\f]

                \param fpVect Source&destination vector
                \param fWeight Weight, w
                \param lLength Vector length
            */
            void Diff (float *, float, long);
            /// \overload
            void Diff (double *, double, long);
            /**
                \overload
                \param fpDest Destination vector
                \param fpSrc Source vector
                \param fWeight Weight, w
                \param lLength Vector length
            */
            void Diff (float *, const float *, float, long);
            /// \overload
            void Diff (double *, const double *, double, long);
            /**
                Inverse differential method (experimental)
            */
            void InvDiff (float *, float, long);
            /// \overload
            void InvDiff (double *, double, long);
            /// \overload
            void InvDiff (float *, const float *, float, long);
            /// \overload
            void InvDiff (double *, const double *, double, long);
    };
    
#endif
