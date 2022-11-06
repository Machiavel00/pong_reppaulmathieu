#include "pong.h"

// Stores hardware peripherals
static Pong_Handle_TypeDef *pong_handle = NULL;

//code à moi
uint32_t tempus_fugit=0;
//code à moi
//cette variable va servir à l'utilisation du temps pour les boucles afin de remplacer le HAL_Delay

// Stores FSM
static FSM_Handle_TypeDef *fsm_handle = NULL;
// FSM enum and callbacks
static FSM_State_TypeDef states_list[] = {
	{STATE_START, &state_start},
	{STATE_WPP1, &state_wpp1},
	{STATE_WPP2, &state_wpp2},
	{STATE_GTP1, &state_gtp1},
	{STATE_GTP2, &state_gtp2}, 	// Go to P2
	{STATE_RPP1, &state_rpp1},		// Reflex press P1
	{STATE_RPP2, &state_rpp2},	// Reflex press P2
	{STATE_IP1S, &state_ip1s },	// Increment P1 score
	{STATE_IP2S, &state_ip2s },		// Increment P1 score
	{STATE_P1WN, &state_p1wn },		// P1 Wins !
	{STATE_P2WN, &state_p2wn },
};
//on crée ici ub tableau d'état que nous allons utiliser
/**
 * @brief Set new FSM state
 * @param _new_state Enum member representing desired state.
 * It will check if desired state is contained in states list.
 */
static void set_new_state(FSM_State_Enum _new_state)
{
	// Check if desired state is contained in states array
	if ((_new_state >= 0) && (_new_state < fsm_handle->states_list_sz))
	{
		// Set new FSM state UID & callback
		fsm_handle->state = fsm_handle->states_list[_new_state];



		// Reset execution count for variables initializations
		fsm_handle->controllers.state_execution_count = 0;

		// Reset animation state
		fsm_handle->controllers.animation_state = ANIMATION_RUNNING;

		// Reset base time for animations
		fsm_handle->controllers.state_base_time = HAL_GetTick();
	}
}

/**
 * @brief Initialize pong game
 * @param _pong_handle Handle to pong peripherals
 * @param _fsm_handle Handle to Pong FSM
 * @retval HAL status
 */
HAL_StatusTypeDef pong_init(Pong_Handle_TypeDef *_pong_handle, FSM_Handle_TypeDef *_fsm_handle)
{
	HAL_StatusTypeDef max7219_status = HAL_OK;
	HAL_StatusTypeDef led_arrray_status = HAL_OK;

	/* Attribute input parameters */
	pong_handle = _pong_handle;
	fsm_handle = _fsm_handle;

	/* Check input parameters */
	CHECK_PONG_PARAMS();

	/* Init hardware peripherals */
	led_array_init(&(pong_handle->led_array));
	/* Init FSM */
	fsm_handle->states_list = states_list;
	fsm_handle->states_list_sz = sizeof(states_list) / sizeof(FSM_State_TypeDef);
	set_new_state(STATE_START);

	return HAL_OK;
}

/**
 * @brief Run pong game, execute FSM callback and check for transition
 * @retval HAL status
 */
HAL_StatusTypeDef pong_run(void)
{
	CHECK_PONG_PARAMS();

	/* RUN STATE */
	// Call associated callback
	fsm_handle->state.state_callback();

	// Increase execution count
	fsm_handle->controllers.state_execution_count += 1;

	/* CHECK TRANSITION */
	switch (fsm_handle->state.state)
	{

	/* Start state */
	case STATE_START:
		set_new_state(STATE_WPP1);
		fsm_handle->controllers.p1_score=0;
		fsm_handle->controllers.p2_score=0;
		// réinitialisation du score
		break;

	/* "Wait for P1 to press PB1" state */
	case STATE_WPP1:
		if(fsm_handle->inputs.nb_press_btn1==1)//on va créer une variable bouton égal à 1 ou à 0, lié à fsm_input, après chaque utilisation de  set_new_state, il reviendra à 0 ?
			{
			fsm_handle->controllers.led_index=0;
			fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
			fsm_handle->controllers.pass_count=0;
			fsm_handle->controllers.led_shift_period=1500;
			fsm_handle->controllers.state_execution_count=0;
			tempus_fugit=HAL_GetTick();
			set_new_state(STATE_GTP2);
			fsm_handle->inputs.nb_press_btn2=0;
			fsm_handle->inputs.nb_press_btn1=0;
			/* pour préparer le lancement du pong, on remet les variables de passe à 0, pour remettre la vitesse par défaut, on remet le bouton à 0 afin qu'il ne considère pas le renvoi de la balle.
			 * dès le lancement, par ailleurs, on veille à ce que le state soit remis en running afin de ne pas clore l'animation trop tôt.
			 * Comme on est dans le wpp1 et que la led s'apprête à partir de droite à gauche, on replace l'index de la led à l'endroit où il est censé s'allumer.
			*/
			}
		break;

	/* "Wait for P2 to press PB2" state */
	case STATE_WPP2:
		if(fsm_handle->inputs.nb_press_btn2==1)
		{
			fsm_handle->controllers.led_index=7;
			fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
			fsm_handle->controllers.pass_count=0;
			fsm_handle->controllers.led_shift_period=1500;
			fsm_handle->controllers.state_execution_count=0;
			tempus_fugit=HAL_GetTick();
			set_new_state(STATE_GTP1);
			fsm_handle->inputs.nb_press_btn1=0;
			fsm_handle->inputs.nb_press_btn2=0;
			/* on réinialise la vitesse, repositionne la led à l'endroit de départ, remet l'animation au lancement, et remet la variable du bouton à l'état 0
			 * On réinialise la variable tempus fugit afin de permettre les conditions de la boucle while dans l'animation suivante.(HAL_GetTick-tempus>temps)
			 * on replace l'éxécution du state execution ainsi que la quantité de passes afin de ne pas implémener la vitesse dès la premiere passe.
			 */
		}
		break;

	/* "Go to P1" state */
	case STATE_GTP1:
		if((fsm_handle->controllers.led_index)==0 && (fsm_handle->controllers.animation_state)==ANIMATION_ENDED)
		{
			fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
			tempus_fugit=HAL_GetTick();
			set_new_state(STATE_RPP1);
		}
		/*lorsque l'on arrive à la dernière led et que celle ci a terminé de s'afficher, alors on rentre dans l'état RPP1,
		 * on prépare la nouvelle animation à l'état running et réajuste la variable de temps pour la boucle while suivante
		 * */
		else if(fsm_handle->inputs.nb_press_btn1==1)
		{
			set_new_state(STATE_IP2S);
			fsm_handle->inputs.nb_press_btn1=0;
		}
		// si le j1 appuie durant l'animation trop tôt alors il donne le point à j2
		else if(fsm_handle->inputs.nb_press_btn2==1)
		{
			fsm_handle->inputs.nb_press_btn2=0;
		}
		//si le j2 appuie involontairement sur le btn2 alors on réinialise le bouton pour éviter les erreur de conditions
		else{
			set_new_state(STATE_GTP1);
		}
		// à chaque led allumée, on revient sur l'état gtp1 pour allumer les leds suivantes jusqu'à la dernière.
		break;

	/* "Go to P2" state */
	case STATE_GTP2:
		if(fsm_handle->controllers.led_index==7 && (fsm_handle->controllers.animation_state)==ANIMATION_ENDED)
			{
				fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
				tempus_fugit=HAL_GetTick();
				set_new_state(STATE_RPP2);
			}
		//si on arrive à la dernière led et que l'animation est terminée, alors on passe à l'état suivant et prépare celui ci
		else if(fsm_handle->inputs.nb_press_btn2==1)
			{
				set_new_state(STATE_IP1S);
				fsm_handle->inputs.nb_press_btn2=0;
			}
		//si le j2 appuie trop tôt, alors il donne le point à j1
		else if(fsm_handle->inputs.nb_press_btn1==1)
			{
				fsm_handle->inputs.nb_press_btn1=0;
			}
		// si le btn1 est pressé involontairement, alors il est réinitialiser pour ne pas causer d'erreur plus tard
		else{
				set_new_state(STATE_GTP2);
			}
		//lorsque l'on a finit d'allumer une led, on passe à la suivante
		break;

	/* "Reflex press P1" state */
	case STATE_RPP1:
		if(fsm_handle->inputs.nb_press_btn1==1 && (fsm_handle->controllers.animation_state)==ANIMATION_ENDED)
		{
			set_new_state(STATE_GTP2);
			fsm_handle->inputs.nb_press_btn1=0;
			fsm_handle->controllers.pass_count++;
			fsm_handle->controllers.state_execution_count=-1;
		}
		/*si l'animation est terminee et que j1 a appuye entre-temps, alors on relance la balle vers j2.
		 * on implémente le pass count afin de raccourcir le temps entre les leds.
		 * On met le state_execution à -1 afin qu'il soit à 0 lorsque l'état GTP2, ainsi on peut valider les conditions d'implémentation de la vitesse dans
		 * GTP2. On n'oublie pas de réinitialiser le btn1
		 */
		else if(fsm_handle->controllers.animation_state==ANIMATION_ENDED)
		{
			set_new_state(STATE_IP2S);
		}// si j1 n'a pas appuyé sur btn1 à temps, alors il donne le point à j2 on implémente donc son score
		else{
			set_new_state(STATE_RPP1);
		}
		//on boucle un certain temps sur l'état RPP1 afin de laisser le temps au joueur d'appuyer sur le bouton
		break;

	/* "Reflex press P2" state */
	case STATE_RPP2:
		if(fsm_handle->inputs.nb_press_btn2==1 && (fsm_handle->controllers.animation_state)==ANIMATION_ENDED)
		{
			fsm_handle->inputs.nb_press_btn2=0;
			fsm_handle->controllers.pass_count++;
			fsm_handle->controllers.state_execution_count=-1;
			set_new_state(STATE_GTP1);
		}
		/*si l'animation est terminee et que j1 a appuye entre-temps, alors on relance la balle vers j1.
				 * on implémente le pass count afin de raccourcir le temps entre les leds.
				 * On met le state_execution à -1 afin qu'il soit à 0 lorsque l'état GTP1, ainsi on peut valider les conditions d'implémentation de la vitesse dans
				 * GTP1. On n'oublie pas de réinitialiser le btn2
				 */
		else if(fsm_handle->controllers.animation_state==ANIMATION_ENDED){
			set_new_state(STATE_IP1S);
		}
		// si j2 n'a pas appuyé sur btn1 à temps, on passe à l'état IP1S pour implémenter le score de j1
		else{
			set_new_state(STATE_RPP2);
		}
		//on boucle un certain temps sur l'état RPP1 afin de laisser le temps au joueur d'appuyer sur le bouton
		break;

	/* "Increase P1 score" state */
	case STATE_IP1S:
		if(fsm_handle->controllers.p1_score>3)
		{
		fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
		tempus_fugit=HAL_GetTick();
		set_new_state(STATE_P1WN);
		}
		/* si jamais le joueur 1 a 4 points, alors on passe à l'état Player1win.
		 * par ailleurs, on réinitialise la variable de temps afin de permettre de boucler suffisamment longtemps sur P1WN
		 * Ce qui est nécessaire pour voir l'animation.
		 */

		else
		{
			fsm_handle->inputs.nb_press_btn2=0;
			fsm_handle->inputs.nb_press_btn1=0;
			set_new_state(STATE_WPP2);
		}
		//dans le cas où j1 n'a pas encore gagné, on donne la main à j2 pour relancer la passe.
		//On réinitialise les boutons afin de ne pas déclencher le GTP1 automatiquement.
		break;
//les animations states vont servir à bloquer la transition tant que animation pas terminé
	/* "Increase P2 score" state */
	case STATE_IP2S:
		if(fsm_handle->controllers.p2_score>3)
		{
			fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
			tempus_fugit=HAL_GetTick();
			set_new_state(STATE_P2WN);
		}
		// si jamais le joueur 2 a 4 points, alors on passe à l'état Player2win.
		//on réinitialise l'état qui sert de condition pour la prochaine transition
		// on fait de meme avec tempus_fugit qui sert au bouclage pendant un certain temps

		else
		{
			fsm_handle->inputs.nb_press_btn1=0;
			fsm_handle->inputs.nb_press_btn2=0;
			fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
			set_new_state(STATE_WPP1);
		}
		//dans le cas où j2 n'a pas encore gagné, on donne la main à j1 pour relancer la passe.
		//On réinitialise les boutons afin de ne pas déclencher le GTP1 automatiquement.
		break;

	/* "P1 Wins !" state */
	case STATE_P1WN:
		if(fsm_handle->controllers.animation_state==ANIMATION_ENDED)
				{
					fsm_handle->controllers.p1_score=0;
					fsm_handle->controllers.p2_score=0;
					set_new_state(STATE_WPP1);
				}
		//On réinitialise les boutons afin de ne pas déclencher le GTP2 automatiquement.
		else
			{
			set_new_state(STATE_P1WN);
			}
		//on reboucle  PW1N durant un certain temps pour que l'animation soit visible.

		break;

	/* "P1 Wins !" state */
	case STATE_P2WN:
		if(fsm_handle->controllers.animation_state==ANIMATION_ENDED)
		{
			fsm_handle->controllers.p1_score=0;
			fsm_handle->controllers.p2_score=0;
			set_new_state(STATE_WPP2);
		}
		//On réinitialise les boutons afin de ne pas déclencher le GTP1 automatiquement.
		else
		{
			set_new_state(STATE_P2WN);
		}
		break;
		//on reboucle  PW2 durant un certain temps pour que l'animation soit visible.

	default:
		set_new_state(STATE_START);
		break;
	}

	return HAL_OK;
}

void state_start(void)
{
	printf("state_start \n");
	//intialiser les variables ex: pass count, score,btn, baset_ime,index, leftshift
}

void state_wpp1(void)
{
	printf("state_wpp1 \n");
	max7219_display_no_decode(0,0b01111111);
	max7219_display_no_decode(1,0b01001111);
	max7219_display_no_decode(2,0b01011111);
	max7219_display_no_decode(3,0b00000110);
}
//on  affiche "BEGI"

void state_wpp2(void)
{
		printf("state_wpp2 \n");
		max7219_display_no_decode(0,0b01111111);
		max7219_display_no_decode(1,0b01001111);
		max7219_display_no_decode(2,0b01011111);
		max7219_display_no_decode(3,0b00000110);
}


void state_gtp1(void)
{
	if(fsm_handle->controllers.state_execution_count==0)
	{
		fsm_handle->controllers.led_shift_period=1500-300*(fsm_handle->controllers.pass_count);
		if(fsm_handle->controllers.led_shift_period<=200)
		{
			fsm_handle->controllers.led_shift_period=200;
		}
		if(fsm_handle->controllers.led_shift_period>1550)
				{
					fsm_handle->controllers.led_shift_period=200;
				}
	}
//on diminue progressivement le temps de passage d'une led à l'autre à chaque fois que l'on est au premier lancement d'une passe


	printf("state_gtp1 \n");
	printf("temps %lu \n",(fsm_handle->controllers.led_shift_period));
	printf("nombre de passes %lu \n",fsm_handle->controllers.pass_count);

	while(HAL_GetTick()-tempus_fugit<(fsm_handle->controllers.led_shift_period))
		{
			write_array(fsm_handle->controllers.led_index,SET);
			//tant que l'on a pas franchis le temps nécessaire la led reste allumée
		}

		write_array(fsm_handle->controllers.led_index,RESET);
		fsm_handle->controllers.led_index--;
		//printf("%lu \n",HAL_GetTick());
		tempus_fugit=HAL_GetTick();
		// on éteint la première led et on passe à la suivante

		if(fsm_handle->controllers.led_index<0)
		{
			fsm_handle->controllers.led_index=0;
			fsm_handle->controllers.animation_state=ANIMATION_ENDED;
			fsm_handle->controllers.state_execution_count=0;
		}
//dans le cas où l'on a allumé la dernière led, l'index est passé à 8, il faut alors le replacer à 7
//puis on déclare la passe terminée

}


void state_gtp2(void)
{
if(fsm_handle->controllers.state_execution_count==0)
{
	fsm_handle->controllers.led_shift_period=1500-300*(fsm_handle->controllers.pass_count);
	if(fsm_handle->controllers.led_shift_period<=200)
	{
		fsm_handle->controllers.led_shift_period=200;
	}
	if(fsm_handle->controllers.led_shift_period>1550)
		{
			fsm_handle->controllers.led_shift_period=200;
		}
}
	// dans ce cas précis, il arrive que la période prenne des valeurs erronnées en rentrant dans les négatifs, dans le cas où la période dépasse les 1500
//on diminue progressivement le temps de passage d'une led à l'autre à chaque fois que l'on est au premier lancement d'une passe
printf("temps %lu \n",(fsm_handle->controllers.led_shift_period));
printf("nombre de passes %d \n",fsm_handle->controllers.pass_count);
//implémente la vitesse


while(HAL_GetTick()-tempus_fugit<(fsm_handle->controllers.led_shift_period))
{

	write_array(fsm_handle->controllers.led_index,SET);

	//tant que l'on a pas franchis le temps nécessaire la led reste allumée
}
write_array(fsm_handle->controllers.led_index,RESET);
fsm_handle->controllers.led_index++;
tempus_fugit=HAL_GetTick();

	if(fsm_handle->controllers.led_index>7)
	{
		fsm_handle->controllers.led_index=7;
		fsm_handle->controllers.animation_state=ANIMATION_ENDED;
		fsm_handle->controllers.state_execution_count=0;
	}

}

void state_rpp1(void)
{
	//printf("state_rpp1 \n");
	fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
	while(HAL_GetTick()-tempus_fugit>500)
	{
		tempus_fugit=HAL_GetTick();
		fsm_handle->controllers.animation_state=ANIMATION_ENDED;
	}
}
/*on boucle pendant au moins 500ms. car quand on atteint les 500ms, la variable tempus a la meme valeur que HAL_getTick et on sort donc de la boucle a
 * et l'animation ended permet de passer à l'état de machine suivant
 */
void state_rpp2(void)
{
	//printf("state_rpp2 \n");
		//printf("rpp2 \n");
		fsm_handle->controllers.animation_state=ANIMATION_RUNNING;
		while(HAL_GetTick()-tempus_fugit>500)
		{
			tempus_fugit=HAL_GetTick();
			fsm_handle->controllers.animation_state=ANIMATION_ENDED;
		}
	//	printf("%d",fsm_handle->inputs.nb_press_btn2);
}
/*on boucle pendant au moins 500ms. car quand on atteint les 500ms, la variable tempus a la meme valeur que HAL_getTick et on sort donc de la boucle a
 * et l'animation ended permet de passer à l'état de machine suivant
 */

void state_ip1s(void)
{
	tempus_fugit=HAL_GetTick();
	fsm_handle->controllers.p1_score += 1;
	max7219_display_no_decode(0,0b0000000);
	while(HAL_GetTick()-tempus_fugit<800)
	{
	max7219_display_decode(1,fsm_handle->controllers.p1_score);
	}
	printf("ip1s %d \n",fsm_handle->controllers.p1_score);
}
//on implémente le score de 1
//on réinitialise l'afficheur par prudence
//on affiche ensuite le score

void state_ip2s(void)
{
	tempus_fugit=HAL_GetTick();
	fsm_handle->controllers.p2_score += 1;
	max7219_display_no_decode(0,0b00000000);
	while(HAL_GetTick()-tempus_fugit<800)
	{
	max7219_display_decode(1,fsm_handle->controllers.p2_score);
	}
	printf("state_ip2s %d \n",fsm_handle->controllers.p2_score);
}
//on implémente le score de 1
//on réinitialise l'afficheur par prudence
//on affiche ensuite le score pendant 800ms

void state_p1wn(void)
{
	printf("state_pw1n \n");
	while(HAL_GetTick()-tempus_fugit>800)
	{
		fsm_handle->controllers.animation_state=ANIMATION_ENDED;
		tempus_fugit=HAL_GetTick();
	}
	max7219_display_no_decode(0,0b01011111);
	max7219_display_no_decode(1,0b01110111);
	max7219_display_no_decode(2,0b01011111);
	max7219_display_no_decode(3,0b01110110);
	}
//on ne sort de l'animation que lorsque l'on a dépassé les 500ms via ANIMATION ENDED
//ici on affiche "GAGN", c'pas facile de faire des mots avec 4 lettres, hein ?

void state_p2wn(void)
{
	printf("state_pw2n \n");
	while(HAL_GetTick()-tempus_fugit>800)
		{
			fsm_handle->controllers.animation_state=ANIMATION_ENDED;
			tempus_fugit=HAL_GetTick();
		}
	max7219_display_no_decode(0,0b01011111);
	max7219_display_no_decode(1,0b01110111);
	max7219_display_no_decode(2,0b01011111);
	max7219_display_no_decode(3,0b01110110);
}
//on ne sort de l'animation que lorsque l'on a dépassé les 500ms via ANIMATION ENDED. L'utilisation d'un simple 'if' aurait été peut être plus intéressant qu'un while.
//ici on affiche "GAGN"

