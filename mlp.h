#ifndef MLP_H_INCLUDED
#define MLP_H_INCLUDED

#include <cmath>
#include <cstdint>
#include <iostream>

#include "layer.h"

class MultilayerPerceptron {
	public:
		MultilayerPerceptron():MultilayerPerceptron(0.25f, 2, 3, 1){}
		MultilayerPerceptron(const float, const unsigned int, const unsigned int, const unsigned int*, const unsigned int);

		~MultilayerPerceptron()
        {
            if(m_hiddenLayers)
            {
                delete [] m_hiddenLayers;
            }
            if(m_outputLayer)
            {
                delete m_outputLayer;
            }
        }

		template<typename T> void train(const T*, const float*);
		template<typename T> float* classify(const T*);

        template<typename T> static float sigmoid(const T pNum)
        {
            return (1.0f/(1.0f+exp(-pNum)));
        }

        static float sigmoid( const float pNum )
        {
            return (1.0f/(1.0f+expf(-pNum)));
        }

        bool writeToFile(const std::string&);
	protected:
		const float m_eta;
        const unsigned int m_maxHiddenLayer;

		Layer *m_hiddenLayers;
		Layer *m_outputLayer;
    private:
};

template<typename T> void MultilayerPerceptron::train(const T* pIn,const float* pTarget)
{
    float *tmpInput         = pIn;
    float **tmpHiddenOutput = new float*[m_maxHiddenLayer];

    for(unsigned int i=0; i<m_maxHiddenLayer; ++i)
    {
        tmpHiddenOutput[i] = new float[m_hiddenLayers[i].m_width];
        for(unsigned int j=0; j<m_hiddenLayers[i].m_width; ++j)
        {
            tmpHiddenOutput[i][j] = 1*m_hiddenLayers[i].m_weights[0][j];
            for(unsigned int k=1; k<m_hiddenLayers[i].m_in+1; ++k)
            {
                tmpHiddenOutput[i][j] += tmpInput[j-1]*m_hiddenLayers[i].m_weights[k][j];
            }
            tmpHiddenOutput[i][j] = sigmoid(tmpHiddenOutput[i][j]);
        }
        tmpInput = tmpHiddenOutput[i];
    }

    float tmpOutOutput [m_outputLayer->m_width];
    for(unsigned int i=0; i<m_outputLayer->m_width; ++i)
    {
        tmpOutOutput[i] = 1*m_outputLayer->m_weights[0][i];
        for(unsigned int j=1; j<m_outputLayer->m_in; ++j)
        {
            tmpOutOutput[i] += tmpInput[j-1] * m_outputLayer->m_weights[j][i];
        }
        tmpOutOutput[i] = sigmoid(tmpOutOutput[i]);
    }

    //Backpropagation
    //error calculation
    float outDelta[m_outputLayer->m_width];
    for(int i=0; i<m_outputLayer->m_width; ++i)
    {
        outDelta[i] = tmpOutOutput[i]*(1-tmpOutOutput[i])*(pTarget[i]-tmpOutOutput[i]);
    }

    //applying delta to reduce error in output layer
    for(int i=0; i<m_outputLayer->m_width; ++i)
    {
        //apply to bias
        m_outputLayer->m_weights[0][i] += m_eta*1*outDelta[i];
        //weights[j+1][] -> skip the constant coeffecient we already did above
        for(int j=0; j<m_outputLayer->m_in; ++j)
        {
            m_outputLayer->m_weights[j+1][i] += m_eta*tmpOutOutput[j]*outDelta[i];
        }
    }

    Layer *previousLayer    = m_outputLayer;
    float **hidDelta        = new float*[m_maxHiddenLayer];
    float *previousDelta    = outDelta;

    //calculate hidden layer error
    for(int k=m_maxHiddenLayer-1;k>0;--k)
    {
        tmpInput = tmpHiddenOutput[k];
        hidDelta[k] = new float[m_hiddenLayers[k].m_width];
        for(int i=0; i<m_hiddenLayers[k].m_width; ++i)
        {
            hidDelta[i] = tmpInput[i]*(1-tmpInput[i]);
            float tmpSum = 0;
            for(int j=0; j<previousLayer->m_width; ++j)
            {
                tmpSum += previousLayer->m_weights[i+1][j]*previousDelta[j];
            }
            hidDelta[i] *= tmpSum;
        }
        previousLayer = m_hiddenLayers[k];
        previousDelta = hidDelta[k];
    }

    //apply delta to weights
    tmpInput = pIn;
    for(int k=0; k<m_maxHiddenLayer;++k)
    {
        for(int i=0; i<m_hiddenLayers[k].m_width; ++i)
        {
            m_hiddenLayers[k-1].m_weights[0][i] += m_eta*1*hidDelta[k][i];
            for(int j=0; j<m_hiddenLayers[k-1].m_width; ++j)
            {
                m_hiddenLayers[k-1].m_weights[j+1][i] += m_eta*tmpInput[j]*hidDelta[k][i];
            }
        }
        tmpInput = tmpHiddenOutput[k];
    }

    for(int i=0; i<m_maxHiddenLayer;++i)
    {
        delete [] hidDelta[i];
        delete [] tmpHiddenOutput[i];
    }
    delete [] hidDelta;
    delete [] tmpHiddenOutput;
}

template<typename T> float* MultilayerPerceptron::classify(const T *pIn)
{
    float hidOutput[m_hidPerceptrons];
    for(int i=0; i<m_hidPerceptrons; ++i)
    {
        //add constant
        hidOutput[i] = 1*m_hidWeights[0][i];
        //sum up all inputs*weightings
        for(int k=1; k<m_inpPerceptrons+1; ++k)
        {
            hidOutput[i] += pIn[k-1] * m_hidWeights[k][i];
        }
        hidOutput[i] = sigmoid(hidOutput[i]);
    }

    float* output = new float[m_outPerceptrons]();
    for(int i=0; i<m_outPerceptrons; ++i)
    {
        output[i] = 1*m_outWeights[0][i];
        for(int j=1; j<m_hidPerceptrons+1; ++j)
        {
            output[i] += hidOutput[j-1] * m_outWeights[j][i];
        }
        output[i] = sigmoid(output[i]);
    }

    return output;
}

#endif // MLP_H_INCLUDED
