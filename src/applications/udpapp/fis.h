/*
 * Stand-alone C codes for fuzzy inference systems.
 * (This file is included in fismain.c)
 * J.-S. Roger Jang, 1994.
 * Copyright 1994-2001 The MathWorks, Inc. 
 */

/*
  * Copyright 1994-2005 The MathWorks, Inc.
 */
#ifndef __FUZZYCONTROL__
#define __FUZZYCONTROL__

#if defined(MATLAB_MEX_FILE)
# define PRINTF mexPrintf
# define DOUBLE real_T
#elif defined(__SIMSTRUC__)
# define PRINTF ssPrintf
# define DOUBLE real_T
#else
# define PRINTF printf
# define DOUBLE double
#endif
#include <vector>

class FuzzYControl
{
        int fis_row_n, fis_col_n;
//FILE	 *output_file;
        void *fis; // controlador difuso
        DOUBLE **dataMatrix, **fisMatrix, **outputMatrix;

        int inicializaSistemaDifuso(const char *fis_file);
        int finalizaSistemaDifuso();
        int evaluarFisOnLine(void *fis, DOUBLE **dataMatrix, DOUBLE **outputMatrix);
    public:
        FuzzYControl(const char *fis_file)
        {
            int ret = inicializaSistemaDifuso(fis_file);
            if (ret != 1)
            {
            	char mi_char;
                printf("Ha ocurrido un error al inicializar el sistema difuso. No se puede continuar");
                printf("\n\nPulse una tecla para terminar ...");
                scanf("%s", &mi_char);
                exit(0);
            }
        }
        ~FuzzYControl()
        {
            finalizaSistemaDifuso();
        }
        bool runFuzzy(const std::vector<double> &inputData,double &out)
        {
            if (inputData.size()!= 3)
            {
                printf("Error fuzzy input");
                exit(0);
            }
            for (int i = 0;i<3;i++)
            {
                dataMatrix[0][i] = inputData[i];
            }
            int ret = evaluarFisOnLine(fis, dataMatrix, outputMatrix);
            if ( ret != 1 )
            {
                return false;
            }
            else
            {
                out = outputMatrix[0][0];
                return true;
            }
        }
};

#endif /* __FIS__ */
