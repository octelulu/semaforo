/* EA076 - Laboratório de Sistemas Embarcados
 *
 * Projeto 1: Semáforo
 *
 * Sistema de semáforos para uma avenida com acionamento de botão sob demanda de travessias de pedestres
 * e estado de segurança noturna.
 *
 * Octavio Salim Moreira RA 156889
 * Luís Filipe Mentem Gomes de Soutello Ra 136712
 *
 * Professor Responsável: Tiago Fernandes Tavares
 */

#include "Cpu.h"
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif

#define tensao_limiar 1200 // Valor de comparação das tensões obtidas no sensor para classificação
#define tempo_seguranca 40 // Tempo mínimo (*125ms) para transição dia/noite

int estado=1;		// Variável de estados da máquina
int *tensao=0;		// Tensões do sensor após conversão AD
int desabilita_botao=0;	// Botão desabilitado - noite e na transição de estados diurnos
int WaitForResult;
int contador_estados=0;	// Tempo de transição entre estados
int contador_escuro=0;
int contador_claro=0;
int dia=1;		// Dia - dia=1; Noite - dia=0

// Rotina de interrupção temporal
void TI1_OnInterrupt(void)
{
	// Medida e conversão da tensão no sensor
	AD1_Measure(WaitForResult);

	/* Máquina de Mealy

	 * Verificação da entrada: tensão do sensor
	 * Verificação do estado atual: dia/noite

	 */
	if (tensao < tensao_limiar && dia==0){
		contador_claro++; // tempo claro
		contador_escuro=0;
	}
	if (tensao > tensao_limiar && dia==1){
		contador_escuro++; // tempo escuro
		contador_claro=0;
	}

	// Caso o tempo de segurança tenha sido ultrapassado
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

	/* Máquina de Moore

	 * Verificação das entradas: botão e tempo

	 */
	if (dia==1){
		if (desabilita_botao==0 && Bit6_GetVal()==0){
			// Botão (Bit 6) pressionado
			desabilita_botao=1;
		}
		// Transições temporais
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
			desabilita_botao=0;	// Habilita botão novamente
		}
	}


	/* Bit1 - led vermelho (carros)
	 * Bit2 - led amarelo (carros)
	 * Bit3 - led verde (carros)
	 * Bit4 - led verde (pedestres)
	 * Bit5 - led vermelho (pedestres)
	 */

	switch(estado){
	// Descrição dos estados (1 a 5)
		case(1):
			Bit1_ClrVal();
			Bit2_ClrVal();
			Bit3_SetVal();
			Bit4_ClrVal();
			Bit5_SetVal();
			// A rotina diurna só começa sob acionamento do botão
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
