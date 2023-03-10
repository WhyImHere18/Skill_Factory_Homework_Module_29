#include <mutex>

struct Node
{
    int value;
    Node* next;
    std::mutex* node_mutex;
    Node(int value) : value(value), next(nullptr), node_mutex(nullptr) {}
};

class FineGrainedQueue
{
    Node* head;
    std::mutex* queue_mutex;
public:
    void insertIntoMiddle(int value, int position);
};

void FineGrainedQueue::insertIntoMiddle(int value, int pos)
{
    Node* newNode = new Node(value);  // создаем новый узел

    int currPos = 0;

    queue_mutex->lock();    // блокируем весь список для поиска узла, за которым нужно вставить новый узел. Если этого не сделать, то другой поток 
                            // может попытаться вызвать функцию вставки для узла, который находится "левее" нашего узла в списке, 
                            // что приведет к его блокированию и нам придется ждать его разблокировки

    // в целом блокировка всей очереди для поиска нужного узла не очень хорошая идея, так как поиск нужного узла в цикле является самой затратной частью алгоритма

    Node* current = head;

    while (currPos < pos - 1 && current->next != nullptr)     // в цикле идем по списку, пока список не кончится, или пока не дойдем до позиции
    {
        current = current->next;
        currPos++;
    }
    current->node_mutex->lock();    // нашли нужный узел, за которым нужно вставить новый узел и блокируем найденный узел
    queue_mutex->unlock();          // разблокируем список (при этом найденный узел все еще заблокирован)

    // Здесь "брешь", так как другой поток может вызвать функцию вставки и это приведет к блокировке всего списка по ходу выполнения этой функции. 
    // Придется ждать разблокировки списка, но далее нет гарантии, что не вклинится еще какой-нибудь поток и придется опять ждать...

    Node* next = current->next;     // меняем указатель на следующий узел на указатель на новый узел.
                                    // При наличии только операции вставки другой поток не может внести изменения в current->next и во все последующие узлы, 
                                    // так как для этого нужно пройти в цикле до этого узла, встретив на пути уже заблокированный узел current 
                                    // и другой поток будет ждать, когда узел current будет разблокирован
    current->next = newNode;
    newNode->next = next;           // связываем список обратно, меняем указатель на узел, следующий после нового узла, на указатель на узел, следующий за current
    current->node_mutex->unlock();  // разблокируем текущий узел. Весь список свободен от блокировок.
}
