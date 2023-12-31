/* Copyright 2014, Mariano Cerdeiro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Gustavo Muro
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief Blinking_echo example source file
 **
 ** This is a mini example of the CIAA Firmware.
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Blinking Blinking_echo example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * MaCe         Mariano Cerdeiro
 * PR           Pablo Ridolfi
 * JuCe         Juan Cecconi
 * GMuro        Gustavo Muro
 * ErPe         Eric Pernia
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20150603 v0.0.3   ErPe change uint8 type by uint8_t
 *                        in line 172
 * 20141019 v0.0.2   JuCe add printf in each task,
 *                        remove trailing spaces
 * 20140731 v0.0.1   PR   first functional version
 */

/*==================[inclusions]=============================================*/
#include "os.h"               /* <= operating system header */
#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
#include "ciaaPOSIX_string.h" /* <= string header */
#include "ciaak.h"            /* <= ciaa kernel header */
#include "myproject.h"         /* <= own header */

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

uint32_t led_rojo = 0x10;
uint32_t led_amarillo = 0x20;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/** \brief File descriptor for digital output ports
 *
 * Device path /dev/dio/out/0
 */
static int32_t fd_out;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */
int main(void)
{
   /* Starts the operating system in the Application Mode 1 */
   /* This example has only one Application Mode */
   StartOS(AppMode1);

   /* StartOs shall never returns, but to avoid compiler warnings or errors
    * 0 is returned */
   return 0;
}

/** \brief Error Hook function
 *
 * This fucntion is called from the os if an os interface (API) returns an
 * error. Is for debugging proposes. If called this function triggers a
 * ShutdownOs which ends in a while(1).
 *
 * The values:
 *    OSErrorGetServiceId
 *    OSErrorGetParam1
 *    OSErrorGetParam2
 *    OSErrorGetParam3
 *    OSErrorGetRet
 *
 * will provide you the interface, the input parameters and the returned value.
 * For more details see the OSEK specification:
 * http://portal.osek-vdx.org/files/pdf/specs/os223.pdf
 *
 */
void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}

/**
 * *************************************************************EXPLICACION*****************************************************
 * En este ejemplo podemos ver como una tarea de menor prioridad puede convertirse en una tarea de mayor prioridad por unos instantes.
 * Podemos ver que inicialmente se ejecuta PeriodicTask, posterior a eso se ejecuta la tarea2 de mas baja prioridad, y en paralelo imediatamente despues
 * se ejecuta la tarea1 de mayor prioridad que la 2, pero podemos observar que la misma no comienza a ejecutarse inmeditatamente
 * ya que se continua ejecutando la tarea 2 ya que se produce la inversion de prioridades.
 */



TASK(InitTask)
{
   /* init CIAA kernel and devices */
   ciaak_start();

   /* print message (only on x86) */
   ciaaPOSIX_printf("Init Task...\n");

   /* open CIAA digital outputs */
   fd_out = ciaaPOSIX_open("/dev/dio/out/0", ciaaPOSIX_O_RDWR);

   /* activate periodic task:
    *  - Se inicia luego de 350 ticks que equivalen a 350ms (definido en HardwareCounter)
    */
   // Tick cada 250ms
   SetRelAlarm(ActivatePeriodicTask, 350, 250);
   SetRelAlarm(ActivatePeriodicTarea1, 400, 250);
   // Tick cada 500ms
   SetAbsAlarm(ActivatePeriodicTarea2, 350, 500);


   /* terminate task */
   TerminateTask();
}

/** \brief Periodic Task
 * Tarea de mayor prioridad
 *
 */
TASK(PeriodicTask)
{
   uint8_t outputs;
   GetResource(Res1);
   /* write blinking message */
   ciaaPOSIX_printf("Blinking\n");

   /* blink output */
   ciaaPOSIX_read(fd_out, &outputs, 1);
   outputs ^= led_rojo;
   ciaaPOSIX_write(fd_out, &outputs, 1);

   /* terminate task */
   ReleaseResource(Res1);
   TerminateTask();
}

/** Tarea con mayor prioridad que tarea2, pero menor que periodic task
 *
 *
 * PD: Para que esta tarea funcione hay que desactivar HOOK_ERRORHOOK ya que dara error porque se ejecuta siempre
 */
TASK(Tarea1) {
 int i =0;
 CancelAlarm(ActivatePeriodicTarea1);
	while(1){
		i++;
		i--;
	}
}

/**
 * Tarea de prioridad minima, la cual se ejecutara luego de periodicTask
 */
TASK(Tarea2)
{
   uint8_t outputs;
   uint32_t ultraDemora = 0;
   GetResource(Res1);

   for(; ultraDemora<10000000; ultraDemora++){
	   ultraDemora++;
	   ultraDemora--;
	   ultraDemora++;
	   ultraDemora--;
   }
   /* write blinking message */
   ciaaPOSIX_printf("Blinking\n");

   /* blink output */
   ciaaPOSIX_read(fd_out, &outputs, 1);
   outputs ^= led_amarillo;
   ciaaPOSIX_write(fd_out, &outputs, 1);

   /* terminate task */
   ReleaseResource(Res1);
   TerminateTask();
}



/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

