/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

CY_ISR_PROTO(Temporizador_ISR);
CY_ISR_PROTO(Pulso_ISR);
CY_ISR_PROTO(Rx_ISR);

void Desconexion(void);
void Visualizacion(void);
void Pulso(void);

uint16 BandPulso=0;
uint16 BandTimer=0;
uint16 BandRx=0;
uint32 Contador=0;
uint32 Contador2=0;
uint16 VALADC=0;
uint16 frecuencia=0;
uint16 vector[6]={0,0,0,0,0,0};
uint16 periodos[5]={0,0,0,0,0};
uint16 frecuencias[5]={0,0,0,0,0};
uint8 T1=0;
uint8 T2=0;
uint8 T3=0;
uint8 T4=0;
uint8 T5=0;
uint16 posicion=0;
uint16 promedio=0;
uint8 bandUna=1;
uint8 bandCero=0;
uint8 BPM=0;



char str[5];


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    amp1_Start();
    amp2_Start();
    PGA_Start();
    Comp_Start();
    Contador_Start();
    ADC_Start();
    LCD_Start();
    Serial_Start();
    Temporizador_StartEx(Temporizador_ISR);
    Pulso_StartEx(Pulso_ISR);
    RXInterrupt_StartEx(Rx_ISR);
    

    for(;;)
    {
        ADC_StartConvert();
        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
        VALADC=ADC_GetResult16();
        
        sprintf(str,"%u",VALADC);
        Serial_PutString(str);
        Serial_PutCRLF(' ');
        CyDelay(1);

        if(Contador>=1500){
            Desconexion();
        }
        
        if (BandPulso){
            Pulso();
        }
    }
}

void Desconexion(void){
    LCD_ClearDisplay();
    LCD_Position(1,4);
    LCD_PrintString("Desconectado");
}

void Pulso(void){
    BandPulso=0;
    //se guarda el tiempo de cada pico de la señal 
    vector[posicion]=Contador;
    posicion++;
    if(bandCero){
        //despues de completar el vector de tiempo la posicion vuelve a ser cero
        posicion=0;
        bandCero=0;
    }
    if (posicion == 6 && bandUna){
        bandCero=1;
        //aqui llega cuando se toman los 6 tiempos 
        bandUna=0;//solo debe entrar la primera vez
        posicion=0;
        //ahora se sacan los periodos  y frecuencias
        T1 = vector[1] - vector[0];
        T2 = vector[2] - vector[1] ;
        T3 = vector[3] - vector[2] ;
        T4 = vector[4] - vector[3] ;
        T5 = vector[5] - vector[4] ;
        
        periodos[0] = T1;
        periodos[1] = T2;
        periodos[2] = T3;
        periodos[3] = T4;
        periodos[4] = T5;
        
        frecuencias[0] = 100/T1;
        frecuencias[1] = 100/T2;
        frecuencias[2] = 100/T3;
        frecuencias[3] = 100/T4;
        frecuencias[4] = 100/T5;
        
        //al terminar el vector inicial debe remplazarse los periodos
        Contador = 0;
    }
    else if(!bandUna){
        //ahora el contador va a ser el mismo periodo y se remplaza en el vcetor
        periodos[posicion] = Contador;
        frecuencias[posicion] = 100/ periodos[posicion];
        Contador = 0;
        
        if(posicion>=5)posicion=0;
        //se calcula el promedio 
        
        promedio = (frecuencias[0] + frecuencias[1] + frecuencias[2] + frecuencias[3] + frecuencias[4])/5;
    }
    if(Contador2>=1000){
        Visualizacion();
    }
}

void Visualizacion(void){
    BPM=promedio;
    LCD_ClearDisplay();
    Contador2=0;
    LCD_Position(1,0);
    LCD_PrintString("F: ");
    LCD_Position(1,6);
    LCD_PrintString("     ");
    LCD_Position(1,3);
    LCD_PrintNumber(promedio/10);
    LCD_PrintString(".");
    LCD_PrintNumber(promedio%10);
    LCD_PrintString("Hz");
    
    LCD_Position(1,10);
    LCD_PrintString("BPM: ");
    LCD_Position(1,15);
    LCD_PrintString("     ");
    LCD_Position(1,15);
    LCD_PrintNumber(BPM*6);
}

CY_ISR(Temporizador_ISR){
    //configurado F=1KHz
    BandTimer=1;
    Contador ++; 
    Contador2++;
}

CY_ISR(Pulso_ISR){
    //configurado F=1KHz
    BandPulso=1;
}

CY_ISR(Rx_ISR){
    //configurado F=1KHz
    BandRx=1;
}
/* [] END OF FILE */
