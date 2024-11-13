/*
Lilian JAOUANNE & Bastian GARCON        18/03/2023
Simulation convertion tension temp�rature thermocouple type K
Source des coefficients : https://aviatechno.net/thermo/table_its90_typek.php
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <math.h>

/* Coefficient Tension -> Temp�rature -------------------------------------------------------------
cV[0][0 � 9] : tensions allants de -5.891 � 0 mV (-200 � 0 �C)
cV[1][0 � 9] : tensions allants de 0 � 20.644 mV (0 � 500 �C)
cV[2][0 � 9] : tensions allants de 20.644 � 54.886 mV (500 � 1372 �C)
*/
double cV[3][10] =
{{
    0,
    2.5173462E+01,
    -1.1662878,
    -1.0833638,
    -8.9773540E-01,
    -3.7342377E-01,
    -8.6632643E-02,
    -1.0450598E-02,
    -5.1920577E-04,
    0
},
{
    0,
    2.508355E+01,
    7.860106E-02,
    -2.503131E-01,
    8.315270E-02,
    -1.228034E-02,
    9.804036E-04,
    -4.413030E-05,
    1.057734E-06,
    -1.052755E-08
},
{
    -1.318058E+02,
    4.830222E+01,
    -1.646031,
    5.464731E-02,
    -9.650715E-04,
    8.802193E-06,
    -3.110810E-08,
    0,
    0,
    0
}};

/* Coefficient Temp�rature -> Tension -------------------------------------------------------------
cC[0][0 � 9] : temp�ratures allants -270 � 0 �C
cC[1][0 � 9] : temp�ratures allants 0 � 1372 �C
*/
double cC[2][11] =
{{
    0,
    0.394501280250E-1,
    0.236223735980E-4,
    -0.328589067840E-6,
    -0.499048287770E-8,
    -0.675090591730E-10,
    -0.574103274280E-12,
    -0.310888728940E-14,
    -0.104516093650E-16,
    -0.198892668780E-19,
    -0.163226974860E-22
},
{
    -0.176004136860E-1,
    0.389212049750E-1,
    0.185587700320E-4,
    -0.994575928740E-7,
    0.318409457190E-9,
    -0.560728448890E-12,
    0.560750590590E-15,
    -0.320207200030E-18,
    0.971511471520E-22,
    -0.121047212750E-25,
    0
}};
// coefficient d'exponentielle pour les temp�ratures sup�rieures � 0
double a[3] = {0.1185976 , -0.1183432E-3 , 0.1269686E+3};

/* Variable servant � la simulation de l'acquisition des donn�es ----------------------------------
*/
    int nbRandInt;
    double nbRandDouble;

/* Fonctions --------------------------------------------------------------------------------------
*/

// renvoie une temp�rature al�atoire comprise entre 10 et 30 �C -----------------------------------
double GetTemp()
{
    // on g�n�re un nombre entier entre 100 et 300
    nbRandInt = (rand() % 201) + 100;
    // on le convertit en "double"
    nbRandDouble = nbRandInt;
    // on le divise par 10 pour que le nombre soit compris entre 10 et 30
    return (nbRandDouble / 10);
}

// renvoie une tension al�atoire comprise entre -6.458 et 54.886 mV -------------------------------
double GetVolt()
{   
    // on g�n�re un nombre entier entre -6458 et 61354
    nbRandInt = (rand() % 61355) -6458;
    // on le convertit en "double"
    nbRandDouble = nbRandInt;
    // on le divise par 10 pour que le nombre soit compris entre 10 et 30
    return (nbRandDouble / 1000);
}

// renvoie la tension en fonction de la temp�rature -----------------------------------------------
double ConvertTemp_Volt(double temp)
{
    double result = 0;
    int i = 0;
    if (temp <= 0)
    {
        for(i=0; i < 11; i++){
            result = result + cC[0][i]*pow(temp , i);
        }
        
    }
    else
    {
        for(i=0; i < 10; i++)
        {
            result = result + cC[1][i]*pow(temp , i); 
        }

        result = result + a[0] * exp(a[1] * pow((temp - a[2]) , 2));
    }
    return(result);
}

// renvoie la temp�rature en fonction de la tension -----------------------------------------------
double ConvertVolt_Temp(double volt)
{
    double result = 0;
    int i = 0;
    
    if (volt <= 0)
    {
        for(i=0; i < 10; i++)
        {
            result = result + cV[0][i]*pow(volt , i); 
        }
        
    }
    else if (volt <= 20.664)
    {
        for(i=0; i < 10; i++)
        {
            result = result + cV[1][i]*pow(volt , i); 
        }
    }
    else
    {
        for(i=0; i < 10; i++)
        {
            result = result + cV[2][i]*pow(volt , i); 
        }
    }
    return(result);
}

void main()
{
    // temp�rature renvoy�e par le capteur de temp�rature
    double tempSensor;
    // tension de compensation de la soudure froide
    double voltSensor;
    // tension mesur�e sur le thermocouple
    double voltThermo;
    // temp�rature thermocouple si la soudure froide �tait � 0 �C (sans compensation soudure froide)
    double tempThermo;
    // temp�rature r�elle
    double tempReel;

    // On commence par initialiser le g�n�rateur de nombre pseudo-al�atoires.
    srand(time(NULL));
    int i = 0;
    for(i=0; i < 100; i++)
    {
        tempSensor = GetTemp();
        voltThermo = GetVolt();

        voltSensor = ConvertTemp_Volt(tempSensor);

        tempThermo = ConvertVolt_Temp(voltThermo);
        tempReel = ConvertVolt_Temp(voltThermo + voltSensor);

        printf("\n |------------------------[Test numero %2d]------------------------|", i);
        printf("\n Capteur de temperature :                      %4.1lf C", tempSensor);
        printf("\n Tension virtuelle du capteur de temperature : %6.3lf mV", voltSensor);
        printf("\n Tension thermocouple :                        %6.3lf mV", voltThermo);
        printf("\n Temperature thermocouple sans compensation :  %6.1lf C", tempThermo);
        printf("\n Temperature thermocouple reelle :             %6.1lf C", tempReel);
    }
}
