#ifndef PONG_PONG_H_
#define PONG_PONG_H_

#include <math.h>
#include "stm32l1xx_hal.h"
#include "led_array.h"
#include "max7219.h"
/*
 * @brief Check that pong handle and fsm handle has been correctly
 * passed (not NULL). If NULL, reset parameters to NULL.
 */
#define CHECK_PONG_PARAMS()          \
	do                               \
	{                                \
		if ((pong_handle == NULL) || \
			(fsm_handle == NULL))    \
		{                            \
			pong_handle = NULL;      \
			fsm_handle = NULL;       \
			return HAL_ERROR;        \
		}                            \
	} while (0)

/**
 * @brief Hardware handlers
 */
typedef struct
{
	TypeDef_LED_Array led_array;
	MAX7219_Handle_TypeDef max7219_handle;
} Pong_Handle_TypeDef;

/**
 * @brief States enumeration, it defines all the states FSM
 * is able to execute.
 */
typedef enum
{
	STATE_START = 0,	// Initial state
	STATE_WPP1 = 1,		// Wait for P1 to press PB1
	STATE_WPP2 = 2,		// Wait for P2 to press PB1
	STATE_GTP1 = 3,		// Go to P1
	STATE_GTP2 = 4,		// Go to P2
	STATE_RPP1 = 5,		// Reflex press P1
	STATE_RPP2 = 6,		// Reflex press P2
	STATE_IP1S = 7,		// Increment P1 score
	STATE_IP2S = 8,		// Increment P1 score
	STATE_P1WN = 9,		// P1 Wins !
	STATE_P2WN = 10,	// P2 Wins !
} FSM_State_Enum;

/**
 * @brief Animation state, used to pass state once
 * animation has ended
 */
typedef enum
{
	ANIMATION_RUNNING, // Animation still running
	ANIMATION_ENDED,   // Animation has ended
} FSM_Animation_Enum;

/**
 * @brief Inputs, used to check for transitions
 */
typedef struct
{
	uint8_t nb_press_btn1; // Count of BTN1 press events
	uint8_t nb_press_btn2; // Count of BTN2 press events
	int state_btn1;
	int state_btn2;
} FSM_Inputs_TypeDef;

/**
 * @brief Structure which links state enum and callback
 * associated to state.
 */
typedef struct
{
	FSM_State_Enum state;	  // Actual state of FSM
	void (*state_callback)(); // Callback to execute : What FSM does at this state
} FSM_State_TypeDef;

/**
 * @brief Controller, stores game variables and state
 * status.
 */
typedef struct
{
	uint32_t state_execution_count;		// Used as reference for variables initializations
	uint32_t state_base_time;			// Used as base for animations
	FSM_Animation_Enum animation_state; // Used to pass state once animation has ended
	uint8_t p1_score;					// P1 score
	uint8_t p2_score;					// P2 score
	int8_t led_index;					// Actual LED index
	uint32_t led_shift_period;			// Period at which LED index is incremented
	uint32_t pass_count;				// Used to store number of pass
} FSM_Controllers_TypeDef;

/**
 * @brief Global handler, stores FSM state, inputs,
 * controllers and list of possible states.
 */
typedef struct
{
	FSM_State_TypeDef state;			 // Actual FSM state
	FSM_Inputs_TypeDef inputs;			 // Inputs states
	FSM_Controllers_TypeDef controllers; // Controllers
	FSM_State_TypeDef *states_list;		 // Array containing associations between FSM_State_Enum and callback
	size_t states_list_sz;				 // Array size
} FSM_Handle_TypeDef;

/* Pong functions */
HAL_StatusTypeDef pong_init(Pong_Handle_TypeDef *_pong_handle, FSM_Handle_TypeDef *_fsm_handle);
HAL_StatusTypeDef pong_run(void);

/* States callbacks */
void state_start(void);
void state_wpp1(void);
void state_wpp2(void);
void state_gtp1(void);
void state_gtp2(void);
void state_rpp1(void);
void state_rpp2(void);
void state_ip1s(void);
void state_ip2s(void);
void state_p1wn(void);
void state_p2wn(void);

#endif /* PONG_PONG_H_ */
