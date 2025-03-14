/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <MLX9064X_I2C_Driver.h>
#include <MLX90641_API.h>
#include <math.h>
 
//------------------------------------------------------------------------------
  
int MLX90641_DumpEE(uint8_t slaveAddr, uint16_t *eeData)
{
     int error = 1;
     error = MLX9064x_I2CRead(slaveAddr, 0x2400, 832, eeData);
     if (error == 0)
     {
        error = MLX90641_HammingDecode(eeData);  
     }
         
     return error;
}

//------------------------------------------------------------------------------

int MLX90641_HammingDecode(uint16_t *eeData)
{
    int error = 0;
    int16_t parity[5];
    int8_t D[16];
    int16_t check;
    uint16_t data;
    uint16_t mask;
    
    for (int addr=16; addr<832; addr++)
    {
        parity[0] = -1;
        parity[1] = -1;
        parity[2] = -1;
        parity[3] = -1;
        parity[4] = -1;
        
        data = eeData[addr];
        mask = 1;
        for( int i = 0; i < 16; i++)
        {          
          D[i] = (data & mask) >> i;
          mask = mask << 1;
        }
        
        parity[0] = D[0]^D[1]^D[3]^D[4]^D[6]^D[8]^D[10]^D[11];
        parity[1] = D[0]^D[2]^D[3]^D[5]^D[6]^D[9]^D[10]^D[12];
        parity[2] = D[1]^D[2]^D[3]^D[7]^D[8]^D[9]^D[10]^D[13];
        parity[3] = D[4]^D[5]^D[6]^D[7]^D[8]^D[9]^D[10]^D[14];
        parity[4] = D[0]^D[1]^D[2]^D[3]^D[4]^D[5]^D[6]^D[7]^D[8]^D[9]^D[10]^D[11]^D[12]^D[13]^D[14]^D[15];
       
        if ((parity[0]!=0) || (parity[1]!=0) || (parity[2]!=0) || (parity[3]!=0) || (parity[4]!=0))
        {        
            check = (parity[0]<<0) + (parity[1]<<1) + (parity[2]<<2) + (parity[3]<<3) + (parity[4]<<4);
    
            if ((check > 15)&&(check < 32))
            {
                switch (check)
                {    
                    case 16:
                        D[15] = 1 - D[15];
                        break;
                    
                    case 24:
                        D[14] = 1 - D[14];
                        break;
                        
                    case 20:
                        D[13] = 1 - D[13];
                        break;
                        
                    case 18:
                        D[12] = 1 - D[12];
                        break;                                
                        
                    case 17:
                        D[11] = 1 - D[11];
                        break;
                        
                    case 31:
                        D[10] = 1 - D[10];
                        break;
                        
                    case 30:
                        D[9] = 1 - D[9];
                        break;
                    
                    case 29:
                        D[8] = 1 - D[8];
                        break;                
                    
                    case 28:
                        D[7] = 1 - D[7];
                        break;
                        
                    case 27:
                        D[6] = 1 - D[6];
                        break;
                            
                    case 26:
                        D[5] = 1 - D[5];
                        break;    
                        
                    case 25:
                        D[4] = 1 - D[4];
                        break;     
                        
                    case 23:
                        D[3] = 1 - D[3];
                        break; 
                        
                    case 22:
                        D[2] = 1 - D[2];
                        break; 
                            
                    case 21:
                        D[1] = 1 - D[1];
                        break; 
                        
                    case 19:
                        D[0] = 1 - D[0];
                        break;     
                                     
                }
               
                if(error == 0)
                {
                    error = -9;
                   
                }
                
                data = 0;
                mask = 1;
                for( int i = 0; i < 16; i++)
                {                    
                    data = data + D[i]*mask;
                    mask = mask << 1;
                }
       
            }
            else
            {
                error = -10;                
            }   
         }
        
        eeData[addr] = data & 0x07FF;
    }
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90641_GetFrameData(uint8_t slaveAddr, uint16_t *frameData)
{
    uint16_t dataReady = 1;
    uint16_t controlRegister1;
    uint16_t statusRegister;
    int error = 1;
    uint8_t cnt = 0;
    uint8_t subPage = 0;
    
    dataReady = 0;
    while(dataReady == 0)
    {
        error = MLX9064x_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
    }   
    subPage = statusRegister & 0x0001;
        
    while(dataReady != 0 && cnt < 5)
    { 
        error = MLX9064x_I2CWrite(slaveAddr, 0x8000, 0x0030);
        if(error == -1)
        {
            return error;
        }
            
        if(subPage == 0)
        { 
            error = MLX9064x_I2CRead(slaveAddr, 0x0400, 32, frameData); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x0440, 32, frameData+32); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x0480, 32, frameData+64); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x04C0, 32, frameData+96); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x0500, 32, frameData+128); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x0540, 32, frameData+160); 
            if(error != 0)
            {
                return error;
            }
        }    
        else
        {
             error = MLX9064x_I2CRead(slaveAddr, 0x0420, 32, frameData); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x0460, 32, frameData+32); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x04A0, 32, frameData+64); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x04E0, 32, frameData+96); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x0520, 32, frameData+128); 
            if(error != 0)
            {
                return error;
            }
            error = MLX9064x_I2CRead(slaveAddr, 0x0560, 32, frameData+160); 
            if(error != 0)
            {
                return error;
            }
        }   
        
        error = MLX9064x_I2CRead(slaveAddr, 0x0580, 48, frameData+192); 
        if(error != 0)
        {
            return error;
        }            
                   
        error = MLX9064x_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
        subPage = statusRegister & 0x0001;      
        cnt = cnt + 1;
    }
    
    if(cnt > 4)
    {
        return -8;
    }    
    
    error = MLX9064x_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    
    frameData[240] = controlRegister1;
    frameData[241] = statusRegister & 0x0001;
    
    if(error != 0)
    {
        return error;
    }
    
    return frameData[241];    
}

//------------------------------------------------------------------------------

int MLX90641_ExtractParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    int error = MLX90641_CheckEEPROMValid(eeData);
    
    if(error == 0)
    {
        MLX90641_ExtractVDDParameters(eeData, mlx90641);
        MLX90641_ExtractPTATParameters(eeData, mlx90641);
        MLX90641_ExtractGainParameters(eeData, mlx90641);
        MLX90641_ExtractTgcParameters(eeData, mlx90641);
        MLX90641_ExtractEmissivityParameters(eeData, mlx90641);
        MLX90641_ExtractResolutionParameters(eeData, mlx90641);
        MLX90641_ExtractKsTaParameters(eeData, mlx90641);
        MLX90641_ExtractKsToParameters(eeData, mlx90641);
        MLX90641_ExtractAlphaParameters(eeData, mlx90641);
        MLX90641_ExtractOffsetParameters(eeData, mlx90641);
        MLX90641_ExtractKtaPixelParameters(eeData, mlx90641);
        MLX90641_ExtractKvPixelParameters(eeData, mlx90641);
        MLX90641_ExtractCPParameters(eeData, mlx90641);
        error = MLX90641_ExtractDeviatingPixels(eeData, mlx90641);  
    }
    
    return error;

}

//------------------------------------------------------------------------------

int MLX90641_SetResolution(uint8_t slaveAddr, uint8_t resolution)
{
    uint16_t controlRegister1;
    int value;
    int error;
    
    value = (resolution & 0x03) << 10;
    
    error = MLX9064x_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    
    if(error == 0)
    {
        value = (controlRegister1 & 0xF3FF) | value;
        error = MLX9064x_I2CWrite(slaveAddr, 0x800D, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90641_GetCurResolution(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int resolutionRAM;
    int error;
    
    error = MLX9064x_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    resolutionRAM = (controlRegister1 & 0x0C00) >> 10;
    
    return resolutionRAM; 
}

//------------------------------------------------------------------------------

int MLX90641_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate)
{
    uint16_t controlRegister1;
    int value;
    int error;
    
    value = (refreshRate & 0x07)<<7;
    
    error = MLX9064x_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error == 0)
    {
        value = (controlRegister1 & 0xFC7F) | value;
        error = MLX9064x_I2CWrite(slaveAddr, 0x800D, value);
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90641_GetRefreshRate(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int refreshRate;
    int error;
    
    error = MLX9064x_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    refreshRate = (controlRegister1 & 0x0380) >> 7;
    
    return refreshRate;
}

//------------------------------------------------------------------------------

void MLX90641_CalculateTo(uint16_t *frameData, const paramsMLX90641 *params, float emissivity, float tr, float *result)
{
    float vdd;
    float ta;
    float ta4;
    float tr4;
    float taTr;
    float gain;
    float irDataCP;
    float irData;
    float alphaCompensated;
    float Sx;
    float To;
    float alphaCorrR[8];
    int8_t range;
    uint16_t subPage;
    float ktaScale;
    float kvScale;
    float alphaScale;
    float kta;
    float kv;
    
    subPage = frameData[241];
    vdd = MLX90641_GetVdd(frameData, params);
    ta = MLX90641_GetTa(frameData, params);    
    ta4 = (ta + 273.15);
    ta4 = ta4 * ta4;
    ta4 = ta4 * ta4;
    tr4 = (tr + 273.15);
    tr4 = tr4 * tr4;
    tr4 = tr4 * tr4;
    
    taTr = tr4 - (tr4-ta4)/emissivity;
    
    ktaScale = pow(2,(double)params->ktaScale);
    kvScale = pow(2,(double)params->kvScale);
    alphaScale = pow(2,(double)params->alphaScale);
    
    alphaCorrR[1] = 1 / (1 + params->ksTo[1] * 20);
    alphaCorrR[0] = alphaCorrR[1] / (1 + params->ksTo[0] * 20);
    alphaCorrR[2] = 1 ;
    alphaCorrR[3] = (1 + params->ksTo[2] * params->ct[3]);
    alphaCorrR[4] = alphaCorrR[3] * (1 + params->ksTo[3] * (params->ct[4] - params->ct[3]));
    alphaCorrR[5] = alphaCorrR[4] * (1 + params->ksTo[4] * (params->ct[5] - params->ct[4]));
    alphaCorrR[6] = alphaCorrR[5] * (1 + params->ksTo[5] * (params->ct[6] - params->ct[5]));
    alphaCorrR[7] = alphaCorrR[6] * (1 + params->ksTo[6] * (params->ct[7] - params->ct[6]));
    
//------------------------- Gain calculation -----------------------------------    
    gain = frameData[202];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = params->gainEE / gain; 
  
//------------------------- To calculation -------------------------------------        
    irDataCP = frameData[200];  
    if(irDataCP > 32767)
    {
        irDataCP = irDataCP - 65536;
    }
    irDataCP = irDataCP * gain;

    irDataCP = irDataCP - params->cpOffset * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    
    for( int pixelNumber = 0; pixelNumber < 192; pixelNumber++)
    {      
        irData = frameData[pixelNumber];
        if(irData > 32767)
        {
            irData = irData - 65536;
        }
        irData = irData * gain;
        
        kta = (float)params->kta[pixelNumber]/ktaScale;
        kv = (float)params->kv[pixelNumber]/kvScale;
            
        irData = irData - params->offset[subPage][pixelNumber]*(1 + kta*(ta - 25))*(1 + kv*(vdd - 3.3));                
    
        irData = irData - params->tgc * irDataCP;
        
        irData = irData / emissivity;
        
        alphaCompensated = SCALEALPHA*alphaScale/params->alpha[pixelNumber];
        alphaCompensated = alphaCompensated*(1 + params->KsTa * (ta - 25));
        
        Sx = alphaCompensated * alphaCompensated * alphaCompensated * (irData + alphaCompensated * taTr);
        Sx = sqrt(sqrt(Sx)) * params->ksTo[2];
        
        To = sqrt(sqrt(irData/(alphaCompensated * (1 - params->ksTo[2] * 273.15) + Sx) + taTr)) - 273.15;
                
        if(To < params->ct[1])
        {
            range = 0;
        }
        else if(To < params->ct[2])   
        {
            range = 1;            
        }   
        else if(To < params->ct[3])
        {
            range = 2;            
        }
        else if(To < params->ct[4])
        {
            range = 3;            
        }
        else if(To < params->ct[5])
        {
            range = 4;            
        }
        else if(To < params->ct[6])
        {
            range = 5;            
        }
        else if(To < params->ct[7])
        {
            range = 6;            
        }
        else
        {
            range = 7;            
        }      
        
        To = sqrt(sqrt(irData / (alphaCompensated * alphaCorrR[range] * (1 + params->ksTo[range] * (To - params->ct[range]))) + taTr)) - 273.15;
        
        result[pixelNumber] = To;
    }
}

//------------------------------------------------------------------------------

void MLX90641_GetImage(uint16_t *frameData, const paramsMLX90641 *params, float *result)
{
    float vdd;
    float ta;
    float gain;
    float irDataCP;
    float irData;
    float alphaCompensated;
    float image;
    uint16_t subPage;
    
    subPage = frameData[241];
    
    vdd = MLX90641_GetVdd(frameData, params);
    ta = MLX90641_GetTa(frameData, params);
    
//------------------------- Gain calculation -----------------------------------    
    gain = frameData[202];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = params->gainEE / gain; 
  
//------------------------- Image calculation -------------------------------------    
    irDataCP = frameData[200];  
    if(irDataCP > 32767)
    {
        irDataCP = irDataCP - 65536;
    }
    irDataCP = irDataCP * gain;

    irDataCP = irDataCP - params->cpOffset * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    
    for( int pixelNumber = 0; pixelNumber < 192; pixelNumber++)
    {
        irData = frameData[pixelNumber];
        if(irData > 32767)
        {
            irData = irData - 65536;
        }
        irData = irData * gain;
        
        irData = irData - params->offset[subPage][pixelNumber]*(1 + params->kta[pixelNumber]*(ta - 25))*(1 + params->kv[pixelNumber]*(vdd - 3.3));
        
        irData = irData - params->tgc * irDataCP;
            
        alphaCompensated = (params->alpha[pixelNumber] - params->tgc * params->cpAlpha);
            
        image = irData*alphaCompensated;
            
        result[pixelNumber] = image;
    }
}

//------------------------------------------------------------------------------

float MLX90641_GetVdd(uint16_t *frameData, const paramsMLX90641 *params)
{
    float vdd;
    float resolutionCorrection;
    
    int resolutionRAM;    
    
    vdd = frameData[234];
    if(vdd > 32767)
    {
        vdd = vdd - 65536;
    }
    resolutionRAM = (frameData[240] & 0x0C00) >> 10;
    resolutionCorrection = pow(2, (double)params->resolutionEE) / pow(2, (double)resolutionRAM);
    vdd = (resolutionCorrection * vdd - params->vdd25) / params->kVdd + 3.3;
    
    return vdd;
}

//------------------------------------------------------------------------------

float MLX90641_GetTa(uint16_t *frameData, const paramsMLX90641 *params)
{
    float ptat;
    float ptatArt;
    float vdd;
    float ta;
    
    vdd = MLX90641_GetVdd(frameData, params);
    
    ptat = frameData[224];
    if(ptat > 32767)
    {
        ptat = ptat - 65536;
    }
    
    ptatArt = frameData[192];
    if(ptatArt > 32767)
    {
        ptatArt = ptatArt - 65536;
    }
    ptatArt = (ptat / (ptat * params->alphaPTAT + ptatArt)) * pow(2, (double)18);
    
    ta = (ptatArt / (1 + params->KvPTAT * (vdd - 3.3)) - params->vPTAT25);
    ta = ta / params->KtPTAT + 25;
    
    return ta;
}

//------------------------------------------------------------------------------

int MLX90641_GetSubPageNumber(uint16_t *frameData)
{
    return frameData[241];    

}    

//------------------------------------------------------------------------------
void MLX90641_BadPixelsCorrection(uint16_t *pixels, float *to, paramsMLX90641 *params)
{   
    float ap[2];
    uint8_t pix;
    uint8_t line;
    uint8_t column;
    
    pix = 0;
    while(pixels[pix]< 65535)
    {
        line = pixels[pix]>>5;
        column = pixels[pix] - (line<<5);
               
        if(column == 0)
        {
            to[pixels[pix]] = to[pixels[pix]+1];            
        }
        else if(column == 1 || column == 14)
        {
            to[pixels[pix]] = (to[pixels[pix]-1]+to[pixels[pix]+1])/2.0;                
        } 
        else if(column == 15)
        {
            to[pixels[pix]] = to[pixels[pix]-1];
        } 
        else
        {            
            ap[0] = to[pixels[pix]+1] - to[pixels[pix]+2];
            ap[1] = to[pixels[pix]-1] - to[pixels[pix]-2];
            if(fabs(ap[0]) > fabs(ap[1]))
            {
                to[pixels[pix]] = to[pixels[pix]-1] + ap[1];                        
            }
            else
            {
                to[pixels[pix]] = to[pixels[pix]+1] + ap[0];                        
            }
                    
        }                      
     
        pix = pix + 1;    
    }    
}

//------------------------------------------------------------------------------

void MLX90641_ExtractVDDParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    int16_t kVdd;
    int16_t vdd25;
    
    kVdd = eeData[39];
    if(kVdd > 1023)
    {
        kVdd = kVdd - 2048;
    }
    kVdd = 32 * kVdd;
    
    vdd25 = eeData[38];
    if(vdd25 > 1023)
    {
        vdd25 = vdd25 - 2048;
    }
    vdd25 = 32 * vdd25;
    
    mlx90641->kVdd = kVdd;
    mlx90641->vdd25 = vdd25; 
}

//------------------------------------------------------------------------------

void MLX90641_ExtractPTATParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float KvPTAT;
    float KtPTAT;
    int16_t vPTAT25;
    float alphaPTAT;
    
    KvPTAT = eeData[43];
    if(KvPTAT > 1023)
    {
        KvPTAT = KvPTAT - 2048;

        ktaScale1 = 0;
        while(temp < 64)
        {
            temp = temp*2;
            ktaScale1 = ktaScale1 + 1;
        }    
         
        for(int i = 0; i < 192; i++)
        {
            temp = ktaTemp[i] * pow(2,(double)ktaScale1);
            if (temp < 0)
            {
                mlx90641->kta[i] = (temp - 0.5);
            }
            else
            {
            mlx90641->kta[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90641->ktaScale = ktaScale1;
}

//------------------------------------------------------------------------------

void MLX90641_ExtractKvPixelParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    uint8_t kvScale1;
    uint8_t kvScale2;
    int16_t kvAvg;
    int16_t tempKv;
    float kvTemp[192];
    float temp;

    kvAvg = eeData[23];
    if (kvAvg > 1023)
    {
        kvAvg = kvAvg - 2048;
    }
  
    kvScale1 = eeData[24] >> 5;
    kvScale2 = eeData[24] & 0x001F;

    for(int i = 0; i < 192; i++)
    {
        tempKv = (eeData[448 + i] & 0x001F);
        if (tempKv > 15)
        {
            tempKv = tempKv - 32;
        }

        kvTemp[i] = tempKv * pow(2,(double)kvScale2);
        kvTemp[i] = kvTemp[i] + kvAvg;
        kvTemp[i] = kvTemp[i] / pow(2,(double)kvScale1);
    }
    
    temp = fabs(kvTemp[0]);
    for(int i = 1; i < 192; i++)
    {
        if (fabs(kvTemp[i]) > temp)
        {
            temp = fabs(kvTemp[i]);
        }
    }
    
    kvScale1 = 0;
    while(temp < 64)
    {
        temp = temp*2;
        kvScale1 = kvScale1 + 1;
    }    
     
    for(int i = 0; i < 192; i++)
    {
        temp = kvTemp[i] * pow(2,(double)kvScale1);
        if (temp < 0)
        {
            mlx90641->kv[i] = (temp - 0.5);
        }
        else
        {
            mlx90641->kv[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90641->kvScale = kvScale1;        
}

//------------------------------------------------------------------------------

void MLX90641_ExtractCPParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float alphaCP;
    int16_t offsetCP;
    float cpKv;
    float cpKta;
    uint8_t alphaScale;
    uint8_t ktaScale1;
    uint8_t kvScale;

    alphaScale = eeData[46];
    
    offsetCP = 32 * eeData[47] + eeData[48];
    if (offsetCP > 32767)
    {
        offsetCP = offsetCP - 65536;
    }
       
    alphaCP = eeData[45];
    if (alphaCP > 1023)
    {
        alphaCP = alphaCP - 2048;
    }
    
    alphaCP = alphaCP /  pow(2,(double)alphaScale);
    
    
    cpKta = eeData[49] & 0x001F;
    if (cpKta > 31)
    {
        cpKta = cpKta - 64;
    }
    ktaScale1 = eeData[49] >> 6;    
    mlx90641->cpKta = cpKta / pow(2,(double)ktaScale1);
    
    cpKv = eeData[50] & 0x001F;
    if (cpKv > 31)
    {
        cpKv = cpKv - 64;
    }
    kvScale = eeData[50] >> 6;
    mlx90641->cpKv = cpKv / pow(2,(double)kvScale);
       
    mlx90641->cpAlpha = alphaCP;
    mlx90641->cpOffset = offsetCP;
}

//------------------------------------------------------------------------------

float MLX90641_GetEmissivity(const paramsMLX90641 *mlx90641)
{
    return  mlx90641->emissivityEE;
}

//------------------------------------------------------------------------------

int MLX90641_ExtractDeviatingPixels(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    uint16_t pixCnt = 0;
    uint16_t brokenPixCnt = 0;

    int warn = 0;
    
    for(pixCnt = 0; pixCnt<3; pixCnt++)
    {
        mlx90641->brokenPixels[pixCnt] = 0xFFFF;
    }
        
    pixCnt = 0;    
    while (pixCnt < 192 && brokenPixCnt < 3)
    {
        if((eeData[pixCnt+64] == 0) && (eeData[pixCnt+256] == 0) && (eeData[pixCnt+448] == 0) && (eeData[pixCnt+640] == 0))
        {
            mlx90641->brokenPixels[brokenPixCnt] = pixCnt;
            brokenPixCnt = brokenPixCnt + 1;
        }    
        
        pixCnt = pixCnt + 1;
        
    } 
    
    if(brokenPixCnt > 1)  
    {
        warn = -3;
    }         
    
    return warn;
       
}
 
 //------------------------------------------------------------------------------
 
 int MLX90641_CheckEEPROMValid(uint16_t *eeData)  
 {
     int deviceSelect;
     deviceSelect = ((uint16_t)eeData[10]) & 0x0040;
     
     if(deviceSelect != 0)
     {
         return 0;
     }
     
     return -7;    
 }