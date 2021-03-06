float sigmoid( const float pNum )
{
    return (1.0f/(1.0f+native_exp(-pNum)));//native_exp might be less accurate but faster than exp
}

/*
    Weights should be stored in rows, not columns
*/

kernel void calcLayer(
    const int preP,
    const int curP,
    global const float *curWeight,
    global const float *curInp,
    const unsigned int inpOffset,
    global float *curOutput
    )
{
    const size_t i = get_global_id(0);
    if(i < curP)
    {
        //offset for weights
        const int weightOffset = i*(preP+1);

        //add constant
        curOutput[i] = 1*curWeight[weightOffset];

        //sum up all inputs*weightings
        for(int j=0; j<preP; ++j)
        {
            curOutput[i] += curInp[j+inpOffset] * curWeight[j+1+weightOffset];
        }
        curOutput[i] = sigmoid(curOutput[i]);
    }
}

kernel void calcLayerDelta(
    const int curP,
    const int nexP,
    global const float *curOutput,
    global const float *nexWeights,
    global const float *nexDelta,
    global float *curDelta
    )
{
    const size_t i = get_global_id(0);
    if(i < curP)
    {
        curDelta[i] = curOutput[i]*(1-curOutput[i]);

        float tmpSum = 0;
        for(int j=0; j<nexP; ++j)
        {
            const int weightOffset = j*(curP+1);
            tmpSum += nexWeights[i+1+weightOffset]*nexDelta[j];
        }
        curDelta[i] *= tmpSum;
    }
}

kernel void calcOutputDelta(
    const int outP,
    global const float *outOutput,
    global const float *target,
    const unsigned int targetOffset,
    global float *outDelta
    )
{
    const size_t i = get_global_id(0);
    if(i < outP)
    {
        outDelta[i] = outOutput[i]*(1-outOutput[i])*(target[i+targetOffset]-outOutput[i]);
    }
}

kernel void applyDelta(
    const int preP,
    const int curP,
    const float eta,
    global const float *curDelta,
    global const float *preOutput,
    const unsigned int preOffset,
    global float *curWeight
    )
{
    const size_t i = get_global_id(0);
    if(i < curP)
    {
        //offset for weights
        const int weightOffset = i*(preP+1);

        curWeight[weightOffset] += eta*1*curDelta[i];
        for(int j=0; j<preP; ++j)
        {
            curWeight[j+1+weightOffset] += eta*preOutput[j+preOffset]*curDelta[i];
        }
    }
}
