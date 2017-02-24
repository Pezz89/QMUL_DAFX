#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

class Compressor
{
    public:
        Compressor(int bufferSize) noexcept : inputBuffer(1,1) {
            this->bufferSize = bufferSize;
            // Allocate a lot of dynamic memory here
            x_g                 .allocate(bufferSize, true);
            x_l                 .allocate(bufferSize, true);
            y_g                 .allocate(bufferSize, true);
            y_l                 .allocate(bufferSize, true);
            c                   .allocate(bufferSize, true);
            yL_prev=0;
            autoTime = false;
            compressorONOFF = false;
            resetAll();
            inputBuffer.setSize(1,bufferSize);
            inputBuffer.clear();
        };

        ~Compressor() noexcept {};

        bool compressorONOFF;
        bool autoTime;
        int bufferSize;



        void compress(AudioSampleBuffer &buffer, int m)
        {
            alphaAttack = exp(-1/(0.001 * sampleRate * tauAttack));
            alphaRelease= exp(-1/(0.001 * sampleRate * tauRelease));
            for (int i = 0 ; i < bufferSize ; ++i)
            {
                //Level detection- estimate level using peak detector
                if (fabs(buffer.getWritePointer(m)[i]) < 0.000001) x_g[i] =-120;
                else x_g[i] =20*log10(fabs(buffer.getWritePointer(m)[i]));
                //Gain computer- static apply input/output curve
                if (x_g[i] >= threshold) y_g[i] = threshold+ (x_g[i] - threshold) / ratio;
                else y_g[i] = x_g[i];
                x_l[i] = x_g[i] - y_g[i];
                //Ballistics- smoothing of the gain
                if (x_l[0]>yL_prev)  y_l[i]=alphaAttack * yL_prev+(1 - alphaAttack ) * x_l[i] ;
                else                 y_l[i]=alphaRelease* yL_prev+(1 - alphaRelease) * x_l[i] ;
                //find control
                c[i] = pow(10,(makeUpGain - y_l[i])/20);
                yL_prev=y_l[i];
            }
        }

        void processSamples(AudioSampleBuffer& samples, int numSamples) {
            threshold = -100;
            ratio = 100;
            if (compressorONOFF) jassertfalse;
            if (compressorONOFF)
            {
                if ( (threshold< 0) )
                {
                    inputBuffer.clear();
                    inputBuffer.addFrom(0,0,samples,0,0,bufferSize,1);
                    // compression : calculates the control voltage
                    compress(inputBuffer,0);
                    // apply control voltage to the audio signal
                    for (int i = 0 ; i < numSamples ; ++i)
                    {
                        samples.getWritePointer(0)[i] *= c[i];
                    }
                    inputBuffer.clear();
                    inputBuffer.addFrom(0,0,samples,0,0,bufferSize,1);
                }
            }
        }

        void resetAll()
        {
                tauAttack=0;tauRelease = 0;
                alphaAttack=0;alphaRelease = 0;
                threshold = 0;
                ratio= 1;
                makeUpGain= 0;
                yL_prev=0;
            for (int i = 0 ; i < bufferSize ; ++i)
            {
                x_g[i] = 0; y_g[i] = 0;
                x_l[i] = 0; y_l[i] = 0;
                c[i] = 0;
            }
        }

        void makeCompressor(
                const int sr,
                const bool cOO
            )
        {
            sampleRate = sr;
            compressorONOFF = cOO;
        }
        //////////////////////////////////////////////
        float getThreshold()
        {
            return threshold;
        }
        float getRatio()
        {
            return ratio;
        }
        float getGain()
        {
            return makeUpGain;//problem?
        }
        float getAttackTime()
        {
            return tauAttack;
        }
        float getReleaseTime()
        {
            return tauRelease;
        }
        ////////////////////////////////////////////////////////
        void setThreshold(float T)
        {
            threshold= T;
        }
        void setGain(float G)
        {
            makeUpGain= G;
        }
        void setRatio(float R)
        {
            ratio= R;
        }
        void setAttackTime(float A)
        {
            tauAttack = A;
        }
        void setReleaseTime(float R)
        {
            tauRelease = R;
        }
        void setSampleRate(float sr)
        {
            sampleRate = sr;
        }

    private:
        AudioSampleBuffer inputBuffer;
        HeapBlock <float> x_g, x_l,y_g, y_l,c;// input, output, control
            // parameters
        float ratio,threshold,makeUpGain,tauAttack,tauRelease,alphaAttack,alphaRelease,yL_prev;
        int nhost;
        int sampleRate;

        template <class T> const T& max( const T& a, const T& b )
        {
          return (a < b) ? b : a;
        }
        int round(float inn)
        {
            if (inn > 0) return (int) (inn + 0.5);
            else return (int) (inn - 0.5);
        }
        JUCE_LEAK_DETECTOR (CrossoverFilter);
};
#endif
