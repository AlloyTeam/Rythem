#include "qirulemanager.h"


Q_GLOBAL_STATIC(QiRuleManager, ruleManager)
QiRuleManager *QiRuleManager::instance(){
    return ruleManager();
}
