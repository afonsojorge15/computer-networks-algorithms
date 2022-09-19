/******************************************************************************\
* Path vector routing protocol.                                                *
\******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "routing-simulator.h"
#define min(a,b) ((a)<(b)?(a):(b))


// Message format to send between nodes.
typedef struct {
    cost_t cost[MAX_NODES];
} message_t;

// State format.
typedef struct {
    cost_t state[MAX_NODES][MAX_NODES];
} state_t;

// Handler for the node to allocate and initialize its state.
void *init_state() {
  state_t *state = (state_t *)calloc(1, sizeof(state_t));
  for (int i = 0; i < MAX_NODES; i++){
    for (int a = 0; a < MAX_NODES; a++){
        state->state[i][a] = COST_INFINITY;
    }
  }
  return state;
}
// Notify a node that a neighboring link has changed cost.
void notify_link_change(node_t neighbor, cost_t new_cost) {

   state_t *current_state = (state_t *) get_state();
  
    for (node_t i = get_first_node(); i <= get_last_node(); i++)
    {
        if (i!= get_current_node())
        {   
            int custo_maior = get_link_cost(i);
            int custo_menor = get_link_cost(i);
            node_t var = i;

            
            for (node_t a = get_first_node(); a <= get_last_node(); a++)
            {   
                if (a!= get_current_node() && get_link_cost(a)!=COST_INFINITY )
                {    
                    custo_menor = min((get_link_cost(a) + current_state->state[a][i]),custo_menor);

                    if (custo_maior > custo_menor )
                    {
                        custo_maior =custo_menor;
                        var = a;
                    }
                }
 
            }
            if(current_state->state[get_current_node()][i] != custo_menor){

                current_state->state[get_current_node()][i] = custo_menor;
                current_state->state[i][get_current_node()] = custo_menor;
                set_route(i,var,current_state->state[get_current_node()][i]);

                for (int c = 0; c < MAX_NODES; c++){
                    if (get_link_cost(c) != COST_INFINITY && c != get_current_node())
                    {
                        message_t *mensagem = (message_t *)malloc(sizeof(message_t));
                        for (int b = 0; b < MAX_NODES; b++){ 
                            if (get_link_cost(c) + current_state->state[c][b] != current_state->state[get_current_node()][b])
                            {   
                                mensagem->cost[b] = current_state->state[get_current_node()][b];
                            }else
                            {
                                mensagem->cost[b] = COST_INFINITY;
                            }
                            
                        }
                        send_message(c, mensagem, sizeof(message_t));

                    }
                }
            } 
        }  
    }
}

// Receive a message sent by a neighboring node.
void notify_receive_message(node_t sender, void *message, int size) {
    message_t *mensagem = (message_t *) message;
    state_t *current_state = (state_t *) get_state();
        
    for (int a = 0; a < MAX_NODES; a++){
        if (a != sender)
        {
            current_state->state[sender][a] = mensagem->cost[a];
        }  
    }
 

    for (node_t i = get_first_node(); i <= get_last_node(); i++)
    {
        if (i!= get_current_node())
        {   
            int custo_maior = get_link_cost(i);
            int custo_menor = get_link_cost(i);
            node_t var = i;

            
            for (node_t a = get_first_node(); a <= get_last_node(); a++)
            {   
                if (a!= get_current_node() && get_link_cost(a)!=COST_INFINITY )
                {    
                    custo_menor = min((get_link_cost(a) + current_state->state[a][i]),custo_menor);

                    if (custo_maior > custo_menor )
                    {   
                        custo_maior =custo_menor;
                        var = a;
                    }
                }
 
            }
            if(current_state->state[get_current_node()][i] != custo_menor){
                current_state->state[get_current_node()][i] = custo_menor;
                current_state->state[i][get_current_node()] = custo_menor;
                set_route(i,var,current_state->state[get_current_node()][i]);


                for (int c = 0; c < MAX_NODES; c++){
                    if (get_link_cost(c) != COST_INFINITY && c != get_current_node())
                    {
                        message_t *mensagem = (message_t *)malloc(sizeof(message_t));

                        for (int b = 0; b < MAX_NODES; b++){ 
                            if (get_link_cost(c) + current_state->state[c][b] != current_state->state[get_current_node()][b])
                            {   
                                mensagem->cost[b] = current_state->state[get_current_node()][b];
                            }else
                            {
                                mensagem->cost[b] = COST_INFINITY;
                            }
                            
                        }
                        send_message(c, mensagem, size);

                    }
                }
            } 

        }  
    }
}
