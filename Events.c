/* EA076 - Laborat�rio de Sistemas Embarcados
 *
 * Projeto 1: Sem�foro
 *
 * Sistema de sem�foros para uma avenida com acionamento de bot�o sob demanda de travessias de pedestres
 * e estado de seguran�a noturna.
 *
 * Octavio Salim Moreira RA 156889
 * Lu�s Filipe Mentem Gomes de Soutello Ra 136712
 *
 * Professor Respons�vel: Tiago Fernandes Tavares
 */

#include "Cpu.h"
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif

#define tensao_limiar 1200 // Valor de compara��o das tens�es obtidas no sensor para classifica��o
#define tempo_seguranca 40 // Tempo m�nimo (*125ms) para transi��o dia/noite

int estado=1;		// Vari�vel de estados da m�quina
int *tensao=0;		// Tens�es do sensor ap�s convers�o AD
int desabilita_botao=0;	// Bot�o desabilitado � noite e na transi��o de estados diurnos
int WaitForResult;
int contador_estados=0;	// Tempo de transi��o entre estados
int contador_escuro=0;
int contador_claro=0;
int dia=1;		// Dia - dia=1; Noite - dia=0

// Rotina de interrup��o temporal
void TI1_OnInterrupt(void)
{
	// Medida e convers�o da tens�o no sensor
	AD1_Measure(WaitForResult);

	/* M�quina de Mealy

	 * Verifica��o da entrada: tens�o do sensor
	 * Verifica��o do estado atual: dia/noite

	 */
	if (tensao < tensao_limiar && dia==0){
		contador_claro++; // tempo claro
		contador_escuro=0;
	}
	if (tensao > tensao_limiar && dia==1){
		contador_escuro++; // tempo escuro
		contador_claro=0;
	}

	// Caso o tempo de seguran�a tenha sido ultrapassado
	if (contador_escuro > tempo_seguranca){
		// Noite
		desabilita_botao=1;
		contador_escuro=0;
		estado=5;	// Estado final noturno
		dia=0;
	}
	if (contador_claro > tempo_seguranca){
		// Dia
		desabilita_botao=0;
		contador_claro=0;
		estado=1;	// Estado inicial diurno
		dia=1;
	}

	/* M�quina de Moore

	 * Verifica��o das entradas: bot�o e tempo

	 */
	if (dia==1){
		if (desabilita_botao==0 && Bit6_GetVal()==0){
			// Bot�o (Bit 6) pressionado
			desabilita_botao=1;
		}
		// Transi��es temporais
		if(contador_estados==20){
			estado=2;
		}
		if(contador_estados==40){
			estado=3;
		}
		if(contador_estados==60){
			estado=4;
		}
		if(contador_estados==80){
			estado=1;		// Retorno ao estado inicial
			contador_estados=0;
			desabilita_botao=0;	// Habilita bot�o novamente
		}
	}


	/* Bit1 - led vermelho (carros)
	 * Bit2 - led amarelo (carros)
	 * Bit3 - led verde (carros)
	 * Bit4 - led verde (pedestres)
	 * Bit5 - led vermelho (pedestres)
	 */

	switch(estado){
	// Descri��o dos estados (1 a 5)
		case(1):
			Bit1_ClrVal();
			Bit2_ClrVal();
			Bit3_SetVal();
			Bit4_ClrVal();
			Bit5_SetVal();
			// A rotina diurna s� come�a sob acionamento do bot�o
			if(desabilita_botao==1) contador_estados++;
			break;
		case(2):
			Bit1_ClrVal();
			Bit2_SetVal();
			Bit3_ClrVal();
			Bit4_ClrVal();
			Bit5_SetVal();
			contador_estados++;
			break;
		case(3):
			Bit1_SetVal();
			Bit2_ClrVal();
			Bit3_ClrVal();
			Bit4_SetVal();
			Bit5_ClrVal();
			contador_estados++;
			break;
		case(4):
			Bit1_SetVal();
			Bit2_ClrVal();
			Bit3_ClrVal();
			Bit4_ClrVal();
			Bit5_NegVal();
			contador_estados++;
			break;
		case(5):
			Bit1_ClrVal();
			Bit2_NegVal();
			Bit3_ClrVal();
			Bit4_ClrVal();
			Bit5_NegVal();
			break;
	}
}

void AD1_OnEnd(void)
{
	AD1_GetValue16(&tensao);

}
