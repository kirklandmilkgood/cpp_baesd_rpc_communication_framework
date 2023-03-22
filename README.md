# cpp_baesd_rpc_communication_framework
以c++實現之分布式服務rpc通訊框架

本專案主要模擬分佈式架構以rpc協定通訊(傳輸呼叫方法、參數)通訊的過程，以數據序列化協議protbuf，搭配zookeeper為rpc服務註冊中心，並使用muduo函數庫來進行網路傳輸
在現今分布式系統相當普及的環境下，如何使呼叫遠端服務像使用本機(本地)服務一樣便捷、高效，這便是rpc協議的用武之地。

＃數據序列與protobuf協議
由於rpc協議須傳輸服務類別(對象)、函數名稱、函數參數，因此如何將這些數據穩固、高效的傳輸到服務所在的主機上，成為rpc協議的核心。一般數據會以tcp協議為基礎進行傳輸，
因此須先將數據轉換成二進制的數據流，這樣的過程稱為serialization(序列化)，序列化的協議主要有：XML, JSON以及protobuf，經protobuf序列化後的數據體積相較其他兩
個協議小很多，從二進制解析回原本數據的速度也較快，具輕量、解析快的特性，本專案也選擇以此協議進行序列化，prtobuf有自己的語法，可經由protoc編譯器將其編譯成c++的形式

# 框架與服務流程

![CamScanner 03-22-2023 11 38_1](https://user-images.githubusercontent.com/128550044/226798707-00e89de1-070c-4f91-9328-51e61fb17367.jpg)

# zookeeper
由於發起呼叫rpc服務需得知具體的服務在哪台主機上，因此需要一個分布式服務配置中心，提供rpc服務的主機(節點)需向配置中心註冊服務，將其ip、port及服務名稱都註冊上去，而
zookeeper就是扮演這當中註冊與配置中心的角色，像上面的架構圖中UserService提供Login和Register兩個函數(服務)，因此服務的提供者(RpcProvider)需將其ip、port以及
兩個函數名稱註冊到zookeeper的server上，當client要呼叫rpc服務時，就去zookeeper server上查詢服務對應的ip和port

# 紀錄檔系統
為了掌握程式運作狀況以及除錯較為容易，本專案也建立了紀錄檔系統，在此單獨建立了一個thread負責將訊息寫入紀錄檔，而要寫入的訊息會被放入一個queue中，由這個thread取出，
當queue中沒有訊息時，thread進行wait(blocking)狀態，再利用condition_variable在queue中有訊息被放入時，喚醒thread
