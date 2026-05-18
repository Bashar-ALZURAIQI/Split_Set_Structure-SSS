#include <iostream>
using namespace std;

// Tek bir elemani temsil eden dugum
struct ItemNode
{
    int value;          // Elemanin deðeri
    ItemNode* next;     // Ayni gruptaki bir sonraki elemani gösterir

    ItemNode(int v) : value(v), next(nullptr) {}
};

// Bir grubu temsil eden duðum
struct GroupNode
{
    int groupId;           // Grubun kimlik numarasi
    int size;              // Gruptaki eleman sayisi
    int capacity;          // Grupta tutulabilecek maksimum eleman sayisi
    ItemNode* itemsHead;   // Gruptaki ilk elemani gosterir
    GroupNode* next;       // Sonraki grubu gosterir

    GroupNode(int id, int cap)
        : groupId(id), size(0), capacity(cap),
          itemsHead(nullptr), next(nullptr) {}
};

// Bölunup birleþebilen grup yapisi
class SplitSetStructure
{
private:
    GroupNode* headGroup;   // Ilk grubu gösterir
    int nextGroupId;        // Yeni grup olustururken kullanilacak id

public:
    // yapici: Baþlangiçta tek bir boþ grup oluþtur
    SplitSetStructure(int initialCapacity)
    {
        headGroup = new GroupNode(0, initialCapacity);//sifir grup id için kullandi
        nextGroupId = 1;
    }

    // Yikici: Tum gruplari ve iclerindeki elemanlari serbest birakir
    ~SplitSetStructure() 
    {
        GroupNode* g = headGroup;//  pointer ilk grubu iþaret etmek için
        while (g != nullptr)// eðer bir gurup var ise
        {
            // Once grubun icindeki tum eleman duðumlerini sil
            ItemNode* it = g->itemsHead;//  pointer grubtaki ilk elemeni iþaret etmek için
            while (it != nullptr) //eðer bir elemen var ise
            {
                ItemNode* tmp = it;//geçeci pointer
                it = it->next;
                delete tmp;
            }
            // Sonra grup dugumunu sil
            GroupNode* gtmp = g;// geçeci pointer
            g = g->next;
            delete gtmp;
        }
    }

    // En az dolu grubu seçmek için yardimci fonksiyon
    GroupNode* chooseGroupForInsert(int value) 
    {
        GroupNode* g = headGroup;
        GroupNode* best = headGroup;

        while (g != nullptr)
        {
            if (g->size < best->size) 
            {
                best = g;
            }
            g = g->next;
        }
        return best;
    }

    // Yeni bir deðer ekleme
    void add(int value)
    {
        // En az dolu grubu bul
        GroupNode* g = chooseGroupForInsert(value);

        // Yeni elemani grubun baþina ekle
        ItemNode* newItem = new ItemNode(value);
        newItem->next = g->itemsHead;
        g->itemsHead = newItem;
        g->size++;

        cout << "Ekle: deger=" << value
             << " -> Grup " << g->groupId
             << " (boyut=" << g->size << ")\n";

        // Kapasite aþildiysa grubu böl
        if (g->size > g->capacity) 
        {
            cout << "Grup " << g->groupId << " kapasiteyi asti, bolunuyor...\n";
            splitGroup(g);
        }
    }

    // Verilen deðeri yapidan silme
    void removeValue(int value) 
    {
        GroupNode* g = headGroup;

        // Tum gruplari tara
        while (g != nullptr) 
        {
            ItemNode* prev = nullptr;
            ItemNode* cur = g->itemsHead;

            // Bu grubun icinde ara
            while (cur != nullptr)
            {
                if (cur->value == value) 
                {
                    // Eleman bulundu, baðlantilari duzelt
                    if (prev == nullptr) 
                    {
                        // Ilk elemani siliyoruz
                        g->itemsHead = cur->next;
                    }
                    else 
                    {
                        prev->next = cur->next;
                    }
                    delete cur;
                    g->size--;

                    cout << "Sil: deger=" << value
                        << " Grup " << g->groupId
                        << " yeni boyut=" << g->size << "\n";

                    // Silmeden sonra kuçuk / boþ gruplari birlestirmeyi dene
                    mergeSmallGroups();
                    return;
                }
                prev = cur;
                cur = cur->next;
            }

            g = g->next;
        }

        cout << "Sil: deger=" << value
            << " bulunamadi.\n";
    }

    // Tum gruplari ve elemanlari yazdir
    void printAll() const
    {
        cout << "\n--- SplitSetStructure Durumu ---\n";

        GroupNode* g = headGroup;
        if (g == nullptr) 
        {
            cout << "Hic grup yok.\n";
            cout << "--------------------------------\n\n";
            return;
        }

        while (g != nullptr) 
        {
            cout << "Grup ID: " << g->groupId
                << " | Eleman Sayisi: " << g->size
                << " / Kapasite: " << g->capacity << "\n";

            cout << "  Elemanlar: ";
            ItemNode* it = g->itemsHead;
            if (it == nullptr)
            {
                cout << "(Bos)";
            }
            else
            {
                while (it != nullptr)
                {
                    cout << it->value;
                    if (it->next != nullptr) cout << " -> ";
                    it = it->next;
                }
            }
            cout << "\n\n";

            g = g->next;
        }

        cout << "--------------------------------\n\n";
    }

private:
    // Bir grup kapasiteyi aþtiðinda iki gruba "dengeli" böl 
    void splitGroup(GroupNode* group) {
        // Yeni grup olustur
        GroupNode* newGroup = new GroupNode(nextGroupId++, group->capacity);
        newGroup->next = group->next;
        group->next = newGroup;

        // Taþinmasi gereken eleman sayisi (yaklaþik yarisi)
        int targetMove = group->size / 2;
        int moved = 0;

        ItemNode* prev = nullptr;
        ItemNode* cur = group->itemsHead;

        // Listenin baþindan itibaren targetMove kadar elemani yeni gruba taþi
        while (cur != nullptr && moved < targetMove) 
        {
            ItemNode* toMove = cur;
            if (prev == nullptr) 
            {
                group->itemsHead = cur->next;
                cur = group->itemsHead;
            }
            else
            {
                prev->next = cur->next;
                cur = prev->next;
            }

            // Yeni grubun baþina ekle
            toMove->next = newGroup->itemsHead;
            newGroup->itemsHead = toMove;

            group->size--;
            newGroup->size++;
            moved++;
        }

        // Eger hic taþima olmadiysa (guvenlik için)
        if (newGroup->size == 0) {
            group->next = newGroup->next;
            delete newGroup;
            cout << "Uyari: Bolme sonucunda yeni grup bos kaldi, grup silindi.\n";
            return;
        }

        cout << "Bolme tamamlandi. Eski Grup "
            << group->groupId << " boyut=" << group->size
            << ", Yeni Grup " << newGroup->groupId
            << " boyut=" << newGroup->size << "\n";
    }

    // Kuçuk veya boþ gruplari birleþtir / temizle
    void mergeSmallGroups() 
    {
        // Once baþlangiçta tamamen boþ grup varsa temizle
        while (headGroup != nullptr &&
            headGroup->size == 0 &&
            headGroup->next != nullptr)
        {
            GroupNode* toDel = headGroup;
            headGroup = headGroup->next;
            delete toDel;
        }

        GroupNode* g = headGroup;

        while (g != nullptr && g->next != nullptr)
        {
            GroupNode* g2 = g->next;

            // Eger ikinci grup tamamen boþsa, direkt sil
            if (g2->size == 0)
            {
                g->next = g2->next;
                delete g2;
                continue;
            }

            int totalSize = g->size + g2->size;
            int cap = g->capacity; // kapasite ayni varsayildi 

            
            // Iki grubun toplam boyutu kapasite eþit veya daha az ise birleþtir
            if (totalSize <= cap ) 
            {
                cout << "Grup " << g->groupId << " ve Grup "
                    << g2->groupId << " birlestiriliyor...\n";

                // g2'deki tum elemanlari g'ye taþi
                ItemNode* it = g2->itemsHead;
                while (it != nullptr)
                {
                    ItemNode* nextIt = it->next;

                    it->next = g->itemsHead;
                    g->itemsHead = it;
                    g->size++;

                    it = nextIt;
                }

                g->next = g2->next;
                delete g2;

                cout << "Birlestirme sonrasi Grup " << g->groupId
                    << " boyut=" << g->size << "\n";
            }
            else {
                g = g->next;
            }
        }
    }
};

int main() {
    // Baslangicta kapasitesi 4 olan bir grup ile yapiyi olustur
    SplitSetStructure sss(4);

    // Ornek eklemeler
    sss.add(5);
    sss.add(8);
    sss.add(11);
    sss.add(14);
    sss.add(2);
    sss.add(17);
     sss.add(18);
    sss.add(13);
    sss.add(15);
    sss.add(19);

    sss.printAll();

    // Bazi elemanlari silelim
    sss.removeValue(8);
    sss.removeValue(2);
    sss.removeValue(11);

    sss.printAll();

    // Daha fazla ekleme
    sss.add(20);
    sss.add(22);
    sss.add(3);
    sss.add(7);
    sss.add(77);
    sss.add(33);
    sss.add(80);
    sss.add(70);
    sss.add(50);
    sss.add(40);
    

    sss.printAll();

    return 0;
}
