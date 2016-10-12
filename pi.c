#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>

pid_t parent_pid;
int received_signals;

//método monte carlo gera pontos aleatotios e ve quantos caem dentro do circulo
//a função n_inside gera os pontos e retorna qtos pontos estao dentro do circulo
int n_inside(int n) {
    int i;
    int inside;
    float x;
    float y;
    int factor = 100000;

    inside = 0;

    for (i = 0; i < n; ++i) {
        x = (rand() % (factor + 1)) / ((float) factor) * (rand() % 2 == 0 ? 1 : -1);
        y = (rand() % (factor + 1)) / ((float) factor) * (rand() % 2 == 0 ? 1 : -1);

        inside += sqrt(x * x + y * y) <= 1.0 ? 1 : 0;
    }

    return inside;
}

//funcao responsável para verificar se é filho simplesmente vendo se o filho tem pid diferente do pai
int is_child() {
    return getpid() != parent_pid;
}
//funcao responsável para verificar se é pai simplesmente vendo se o processo tem pid igual ao do pai
int is_parent() {
    return getpid() == parent_pid;
}

//função responsável por criar os filhos
pid_t create_child() {
    if (is_parent()) {
        return fork();
    }
}
/*Os sinais em linux funcionam assim: vc diz qual sinal o programa espera receber e trata. Quem trata o sinal eh uma funcao. Neste caso, Ela so faz decrementar um contador que indica quantos sinais ja foram recebidos. 
*/
void signal_handler(int sig) {
    printf("Received signal\n");
    --received_signals;
}

//funcionamento geral 
/*

O pai gera n filhos                         
Cada filho gera uma quantidade aleatoria de pontos e ve qtos estao dentro do circulo                         
Salva o resultado da contagem num arquivo                         
E envia um sinal para o pai dizendo que terminou de processar                         
Quando o pai recebe o sinal                         
Essa funcao signal_handler eh executada                         
Decrementando o contador                         
Quando o contador chega a zero o pai sabe q todos os filhos terminaram de processar                         
Entao ele abre os arquivos gerados pelos filhos                         
Faz a leitura dos valores                         
E o calculo do pi

*/
float pi(int n_iterations, int n_children) {
    int i;
    int inside;
    int tmp;
    int n_iterations_per_children;
    FILE* output;
    pid_t children_pids[255];
    char filename_buffer[255];

    n_iterations_per_children = n_iterations / n_children;
    received_signals = n_children;
    inside = 0;
    parent_pid = getpid();

    for (i = 0; i < n_children; ++i) {
        children_pids[i] = create_child();
    }

    if (is_parent()) {
        signal(SIGUSR1, signal_handler);
    }

    if (is_child()) {
        sprintf(filename_buffer, "/tmp/%i.pi", getpid());
        output = fopen(filename_buffer, "w");
        fprintf(output, "%i\n", n_inside(n_iterations_per_children));
        fclose(output);
        kill(parent_pid, SIGUSR1);
        exit(0);
    } else {
        while (received_signals != 0) { }

        for (i = 0; i < n_children; ++i) {
            sprintf(filename_buffer, "/tmp/%i.pi", children_pids[i]);
            output = fopen(filename_buffer, "r");
            fscanf(output, "%i", &tmp);
            inside += tmp;
        }

        return 4 * ((float) inside / n_iterations);
    }

    return 0.0;
}

void clear_previous_results() {
    system("rm -rf /tmp/*.pi");
}

int main(int argc, char* argv[]) {
    int n_iterations;
    int n_children;

    srand(time(NULL));

    printf("Number of iterations: ");
    scanf("%i", &n_iterations);

    printf("Number of children: ");
    scanf("%i", &n_children);

    clear_previous_results();
    printf("%f\n", pi(n_iterations, n_children));

    return 0;
}
