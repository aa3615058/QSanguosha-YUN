#include "settings.h"
#include "standard.h"
#include "skill.h"
#include "yun.h"
#include "client.h"
#include "engine.h"
#include "ai.h"
#include "general.h"
#include "clientplayer.h"
#include "clientstruct.h"
#include "wrapped-card.h"
#include "room.h"
#include "roomthread.h"

class Tiancheng : public TriggerSkill {
public:
     Tiancheng() : TriggerSkill("tiancheng") {
         events << CardResponded << CardUsed << FinishJudge << EventPhaseChanging;
         frequency = Frequent;
     }
     bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
     {
        const Card *card;
        const QString tianchengKeepMark = "@hongyantiancheng";
        const QString tianchengOddFlag = "tianchengOdd";

        if (triggerEvent == CardResponded) {
            card = data.value<CardResponseStruct>().m_card;
        } else if(triggerEvent == CardUsed) {
            card = data.value<CardUseStruct>().card;
        } else if(triggerEvent == FinishJudge) {
            card = data.value<JudgeStruct *>()->card;
        } else if(triggerEvent == EventPhaseChanging) {
            if(data.value<PhaseChangeStruct>().to == Player::NotActive) {
                player->loseAllMarks(tianchengKeepMark);
            }
            return false;
        }

        int count = 0;
        if(card->getSkillName() == "hongyan") {
            count++;
        }
        else if (!(card->isKindOf("SkillCard")) && card->subcardsLength() > 0)  {
            QList<int> subcards = card->getSubcards();
            foreach (int cardID, subcards) {
                if(Sanguosha->getCard(cardID)->getSkillName() == "hongyan") {
                    count++;
                }
            }
        }

        for(int i = 0; i < count; i++) {
            if(player->askForSkillInvoke(this)) {
                bool drawFlag = true;
                if(player->getPhase() == Player::Play) {
                    if(player->hasFlag(tianchengOddFlag)) {
                        drawFlag = false;
                        player->gainMark(tianchengKeepMark);
                        room->setPlayerFlag(player, "-"+tianchengOddFlag);
                    } else {
                        player->setFlags(tianchengOddFlag);
                    }
                }
                if(drawFlag) {
                    player->drawCards(1, objectName());
                }
            }
        }
        return false;
     }
};

class TianchengKeep : public MaxCardsSkill {
public:
    TianchengKeep() : MaxCardsSkill("#tianchengKeep") {
        frequency = Frequent;
    }

    int getExtra(const Player *target) const {
        if (target->hasSkill(this))
            return target->getMark("@hongyantiancheng");
        else
            return 0;
    }
};

class Lianji : public TriggerSkill {
public:
     Lianji() : TriggerSkill("lianji") {
         events << Damage;
     }

     bool triggerable(const ServerPlayer *target) const {
         return target != NULL;
     }

     bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const {
        ServerPlayer* victim = data.value<DamageStruct>().to;
        QList<ServerPlayer *> jingmeizis;
        foreach (ServerPlayer *player, room->getAllPlayers()) {
            if (player->hasSkill(this) && player->isAlive() && player->getPhase() == Player::NotActive && player->canSlash(victim) && !(player->isNude())) {
                jingmeizis.append(player);
            }
        }
        foreach(ServerPlayer *jingmeizi, jingmeizis) {
            if (room->askForUseSlashTo(jingmeizi, victim, "@lianji-prompt")) {
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = jingmeizi;
                log.arg = objectName();
                room->sendLog(log);
                jingmeizi->drawCards(1, objectName());
            }
        }
        return true;
     }
};

QiaopoCard::QiaopoCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}
void QiaopoCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *target = effect.to;
    DamageStruct damage = effect.from->tag.value("QiaopoDamage").value<DamageStruct>();
    target->obtainCard(this);
    DamageStruct damage2("qiaopo", damage.from, target, 1, damage.nature);
    damage2.transfer=true;
    damage2.transfer_reason="qiaopo";
    //effect.from->tag["TransferDamage"] = QVariant::fromValue(damage2);
    target->getRoom()->damage(damage2);
}
class QiaopoViewAsSkill : public OneCardViewAsSkill {
public:
    QiaopoViewAsSkill() : OneCardViewAsSkill("qiaopo") {
        filter_pattern = ".|diamond";
        response_pattern = "@@qiaopo";
    }
    const Card *viewAs(const Card *originalCard) const {
        QiaopoCard *qiaopoCard = new QiaopoCard;
        qiaopoCard->addSubcard(originalCard);
        return qiaopoCard;
    }
};
class Qiaopo : public TriggerSkill {
public:
    Qiaopo() : TriggerSkill("qiaopo") {
        events << DamageInflicted;
        view_as_skill = new QiaopoViewAsSkill;
    }
    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (player->canDiscard(player, "he")) {
            player->tag["QiaopoDamage"] = data;
            DamageStruct damage = data.value<DamageStruct>();
            int m = damage.damage;
            int x = 0;
            for(int i = 0; i < m; i++) {
                if(room->askForUseCard(player, "@@qiaopo", "@qiaopo-card")) {
                    x++;
                }else {
                    break;
                }
            }
            if(x == m) {
                return true;
            }else {
                damage.damage = m - x;
                data.setValue(damage);
            }
        }
        return false;
    }
};

class Siwu : public TriggerSkill
{
public:
    Siwu() : TriggerSkill("siwu") {
        events << Damaged << EventLoseSkill << EventAcquireSkill;
        frequency = Frequent;
    }

    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(triggerEvent == Damaged && player->hasSkill(this)) {
            for(int i = 0; i < data.value<DamageStruct>().damage; i++) {
                if(player->isAlive() && player->askForSkillInvoke(this)) {
                    player->drawCards(1, objectName());
                    if (!(player->isKongcheng())) {
                        int cardID;
                        if(player->getHandcardNum() == 1) {
                            cardID = player->handCards().first();
                        } else {
                            cardID = room->askForExchange(player, objectName(), 1, 1, false, "@siwu-prompt")->getEffectiveId();
                        }
                        player->addToPile("&wire", cardID, false);
                    }
                }
            }
        }
        else if(triggerEvent == EventLoseSkill && data.toString() == objectName()) {
            player->pileToPile("&wire", "wire", false);
        } else if(triggerEvent == EventAcquireSkill && data.toString() == objectName()) {
            player->pileToPile("wire", "&wire", false);
        }
        return false;
    }
};

XingcanCard::XingcanCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}
bool XingcanCard::targetFilter(const QList<const Player *> &, const Player *, const Player *) const {
    return true;
}
void XingcanCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer * player = effect.from;
    ServerPlayer * target = effect.to;
    Room *room = player->getRoom();
    target->obtainCard(this);
    player->drawCards(1, "xingcan");
    if(player != target) {
        QString result = room->askForChoice(player,"xingcan","xingcan_use+xingcan_lockhandcard");
        if (result == "xingcan_use") {
            QString pattern;
            if(target->isWounded()) {
                pattern = this->getEffectiveId()+"+^Nullification+^Jink|.|.|.";
            } else {
                pattern = this->getEffectiveId()+"+^Nullification+^Jink+^Peach|.|.|.";
            }
            room->askForUseCard(target, pattern, "@xingcan", -1, Card::MethodUse);
        } else if(result == "xingcan_lockhandcard") {
            if (target->getMark("@ningxingcanyu") == 0) {
                target->gainMark("@ningxingcanyu");
                room->setPlayerCardLimitation(target, "use,response", ".|.|.|hand", true);
            }
        }
    }
}

class XingcanViewAsSkill : public OneCardViewAsSkill {
public:
    XingcanViewAsSkill() : OneCardViewAsSkill("xingcan") {
        filter_pattern = ".|.|.|&wire,wire";
        expand_pile = "&wire,wire";
    }
    bool isEnabledAtPlay(const Player *player) const {
        return !player->hasUsed("XingcanCard") && (!(player->getPile("&wire").empty()) || !(player->getPile("wire").empty()));
    }
    const Card *viewAs(const Card *originalCard) const {
        XingcanCard *xingcanCard = new XingcanCard;
        xingcanCard->addSubcard(originalCard);
        return xingcanCard;
    }
};

class Xingcan : public TriggerSkill {
public:
    Xingcan() : TriggerSkill("xingcan") {
        events << EventPhaseChanging << EventLoseSkill << Death << EnterDying << QuitDying;
        view_as_skill = new XingcanViewAsSkill;
    }
    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }
    void removeXingcanMarkAndLimitation(Room *room) const {
        foreach(ServerPlayer * player, room->getAllPlayers()) {
            if(player->getMark("@ningxingcanyu") > 0) {
                player->loseMark("@ningxingcanyu");
                room->removePlayerCardLimitation(player, "use,response", ".|.|.|hand$1");
            }
        }
    }
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(triggerEvent == EnterDying && player->getMark("@ningxingcanyu") > 0) {
            room->removePlayerCardLimitation(player, "use,response", ".|.|.|hand$1");
        }else if(triggerEvent == QuitDying && player->getMark("@ningxingcanyu") > 0) {
            room->setPlayerCardLimitation(player, "use,response", ".|.|.|hand", true);
        }else if(triggerEvent == EventPhaseChanging) {
            if(data.value<PhaseChangeStruct>().to == Player::NotActive && player->hasSkill(this)) {
                removeXingcanMarkAndLimitation(room);
            }
        }else if(triggerEvent == Death) {
            ServerPlayer *player = data.value<DeathStruct>().who;
            if(player->getPhase() == Player::Play && player->hasSkill(this)) {
                removeXingcanMarkAndLimitation(room);
            }
        }else if(triggerEvent == EventLoseSkill) {
            if(player->getPhase() == Player::Play && data.toString() == objectName()){
                removeXingcanMarkAndLimitation(room);
            }
        }
        return false;
    }
};

class Zhangui : public TriggerSkill {
public:
    Zhangui() : TriggerSkill("zhangui")
    {
        events << CardUsed;
    }
    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(data.value<CardUseStruct>().card->isKindOf("Slash") && player->getPhase() == Player::Play && !(player->hasFlag("slashUsed"))) {
            room->setPlayerFlag(player, "slashUsed");
        }
        return false;
    }
};

class Xiaohan : public TriggerSkill {
public:
    Xiaohan() : TriggerSkill("xiaohan") {
        events << DamageCaused;
    }
    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.nature == DamageStruct::Thunder
            && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0
            && !damage.to->isNude() && damage.by_user
            && player->askForSkillInvoke("xiaohan-ice_sword")) {

            room->broadcastSkillInvoke(objectName());
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            if (damage.from->canDiscard(damage.to, "he")) {
                int card_id = room->askForCardChosen(player, damage.to, "he", objectName(), false, Card::MethodDiscard);
                room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);
                if (damage.from->isAlive() && damage.to->isAlive() && damage.from->canDiscard(damage.to, "he")) {
                    card_id = room->askForCardChosen(player, damage.to, "he", objectName(), false, Card::MethodDiscard);
                    room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);
                }
            }
            return true;
        }
        return false;
    }
};

MiyuCard::MiyuCard(){}
bool MiyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.length() < (Self->getLostHp() / 2 + Self->getLostHp() % 2) && to_select != Self;
}
void MiyuCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *target = effect.to;
    Room *room = target->getRoom();
    Card *lightning = Sanguosha->cloneCard("Lightning", Card::NoSuit, 0);

    lightning->setSkillName("miyu");
    lightning->deleteLater();
    CardEffectStruct lightningEffect;
    lightningEffect.card = lightning;
    lightningEffect.from = effect.from;
    lightningEffect.to = target;
    room->cardEffect(lightningEffect);
}
class MiyuViewAsSkill : public ZeroCardViewAsSkill {
public:
    MiyuViewAsSkill() : ZeroCardViewAsSkill("miyu") {
        response_pattern = "@@miyu";
    }
    const Card *viewAs() const {
        return new MiyuCard;
    }
};
class Miyu : public PhaseChangeSkill {
public:
    Miyu() : PhaseChangeSkill("miyu") {
        view_as_skill = new MiyuViewAsSkill;
    }

    bool onPhaseChange(ServerPlayer *player) const {
        if (player->getPhase() == Player::Finish && player->isWounded()) {
            player->getRoom()->askForUseCard(player, "@@miyu", "@miyu");
        }
        return false;
    }
};

class YingzhouViewAsSkill : public OneCardViewAsSkill {
public:
    YingzhouViewAsSkill() : OneCardViewAsSkill("yingzhou") {
        filter_pattern = ".|.|.|hand";
    }
    bool isEnabledAtPlay(const Player *player) const {
        return player->hasFlag("yingzhouCanInvoke") and !(player->hasFlag("yingzhouInvoked"));
    }
    const Card *viewAs(const Card *originalCard) const {
        Card* acard = Sanguosha->cloneCard(Self->property("yingzhouCard").toString(), originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName(objectName());
        return acard;
    }
    bool isEnabledAtResponse(const Player *player, const QString &) const {
        return !(player->hasFlag("yingzhouInvoked")) && (player->hasFlag("yingzhouNullFication") || player->hasFlag("yingzhouBasicCard"));
    }
    bool isEnabledAtNullification(const ServerPlayer *player) const {
        return !(player->hasFlag("yingzhouInvoked")) && player->hasFlag("yingzhouNullFication");
    }
};

class Yingzhou : public TriggerSkill {
public:
    Yingzhou() : TriggerSkill("yingzhou")
    {
        events << CardUsed << CardResponded << EventPhaseChanging;
        view_as_skill = new YingzhouViewAsSkill;
    }
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(triggerEvent == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::Play) {
            //for liaohua
            room->setPlayerFlag(player, "-yingzhouInvoked");
            room->setPlayerFlag(player, "-yingzhouCanInvoke");
            room->setPlayerFlag(player, "-yingzhouNullFication");
            room->setPlayerFlag(player, "-yingzhouBasicCard");
        }else if(player->getPhase() == Player::Play && !(player->hasFlag("yingzhouInvoked"))) {
            const Card *card = NULL;
            if(triggerEvent == CardResponded) {
                card = data.value<CardResponseStruct>().m_card;
            }else if(triggerEvent == CardUsed) {
                card = data.value<CardUseStruct>().card;
            }
            if(card) {
                if(card->getSkillName() == objectName()) {
                    room->setPlayerFlag(player, "yingzhouInvoked");
                }else {
                    if(card->isNDTrick() || card->isKindOf("BasicCard")) {
                        room->setPlayerFlag(player, "yingzhouCanInvoke");
                        room->setPlayerProperty(player, "yingzhouCard", QVariant(card->getClassName()));

                        if(card->objectName() == "nullification") {
                            room->setPlayerFlag(player, "yingzhouNullFication");
                        } else if(card->isKindOf("BasicCard")) {
                            room->setPlayerFlag(player, "yingzhouBasicCard");
                        }
                    }else if(card->isKindOf("EquipCard") || card->isKindOf("TrickCard")) {
                        room->setPlayerFlag(player, "-yingzhouCanInvoke");
                    }
                }
            }
        }
        return false;
    }
};

QifengCard::QifengCard(){}
bool QifengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.length() < 1 && !(to_select->hasFlag("qifeng_original")) && to_select != Self;
}
void QifengCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *target = effect.to;
    Room *room = target->getRoom();
    QVariant data = room->getTag("qifengdamage");
    DamageStruct damage = data.value<DamageStruct>();

    bool invoke_transfer = false;
    if (target->canDiscard(target, "h")) {
        if (!(room->askForDiscard(target, "qifeng", 1, 1, true, true, "@qifeng-discard", ".|red"))) {
            invoke_transfer = true;
        }
    }
    else {
        invoke_transfer = true;
    }
    if (invoke_transfer) {
        DamageStruct damage2("qifeng", damage.from, target, damage.damage, damage.nature);
        damage2.transfer = true;

        damage.damage = 0;
        data.setValue(damage);
        room->setTag("qifengdamage", data);

        room->damage(damage2);
    }
}
class QifengViewAsSkill : public ZeroCardViewAsSkill {
public:
    QifengViewAsSkill() : ZeroCardViewAsSkill("qifeng") {
        response_pattern = "@@qifeng";
    }
    const Card *viewAs() const {
        return new QifengCard;
    }
};
class Qifeng : public TriggerSkill {
public:
    Qifeng() : TriggerSkill("qifeng") {
        events << DamageCaused;
        view_as_skill = new QifengViewAsSkill;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer* target = damage.to;
        bool transfer = false;
        if (damage.transfer == false) {
            bool can_invoke = false;
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            players.removeOne(target);
            can_invoke = (players.length() > 0) && (target != player);
            if (can_invoke) {
                room->setTag("qifengdamage", data);
                room->setPlayerFlag(damage.to, "qifeng_original");
                if (room->askForUseCard(player, "@@qifeng", "@qifeng")) {
                    if (room->getTag("qifengdamage").value<DamageStruct>().damage == 0) {
                        transfer = true;
                    }
                }
                room->setPlayerFlag(damage.to, "-qifeng_original");
            }
        }
        return transfer;
    }
};

YunPackage::YunPackage()
    :Package("yun")
{
    General *huaibeibei = new General(this, "huaibeibei", "wu", 4, false); // Yun 006
    huaibeibei->addSkill("hongyan");
    huaibeibei->addSkill(new Tiancheng);
    huaibeibei->addSkill(new TianchengKeep);
    related_skills.insertMulti("tiancheng", "#tianchengKeep");

    General *hanjing = new General(this, "hanjing", "wu", 3, false); // Yun 007
    hanjing->addSkill(new Lianji);
    hanjing->addSkill(new Qiaopo);

    General *wangcan = new General(this, "wangcan", "wei", 3, false); // Yun 008
    wangcan->addSkill(new Siwu);
    wangcan->addSkill(new Xingcan);

    General *yangwenqi = new General(this, "yangwenqi", "shu", 4, false); // Yun 009
    yangwenqi->addSkill(new Zhangui);
    //yangwenqi->addSkill(new Diaolue);

    General *xiaosa = new General(this, "xiaosa", "wei", 4, false); // Yun 010
    xiaosa->addSkill(new Xiaohan);
    xiaosa->addSkill(new Miyu);

    General *lishuyu = new General(this, "lishuyu", "shu", 3, false); // Yun 011
    lishuyu->addSkill(new Yingzhou);
    lishuyu->addSkill(new Qifeng);

    addMetaObject<QiaopoCard>();
    addMetaObject<XingcanCard>();
    addMetaObject<MiyuCard>();
    addMetaObject<QifengCard>();
}

ADD_PACKAGE(Yun)

class Lanyan : public TriggerSkill{
public:
    Lanyan() : TriggerSkill("lanyan") {
        events << GameStart << EventPhaseStart << EventPhaseChanging << EventAcquireSkill << EventLoseSkill;
        frequency = Compulsory;
    }
    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }
    void EXliyunpengImageChanged(QString name, ServerPlayer* player, Room *room) const {
        if (player->getGeneralName() == name || player->getGeneral2Name() == name) {
            return;
        }
        if (name == "EXliyunpeng_female") {
            LogMessage log;
            /*log.type = "#InvokeSkill";
            log.from = player;
            log.arg = "lanyan";
            room->sendLog(log);*/
            log.type = "#lanyan";
            log.from = player;
            log.arg = "lanyan";
            log.arg2 = "female";
            room->sendLog(log);
        }
        if (player->getGeneralName() == "EXliyunpeng" || player->getGeneralName() == "EXliyunpeng_female") {
            room->changeHero(player, name, false, false, false, false);
        }
        if (player->getGeneral2Name() == "EXliyunpeng" || player->getGeneral2Name() == "EXliyunpeng_female") {
            room->changeHero(player, name, false, false, true, false);
        }
    }
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
      if (triggerEvent == EventPhaseStart && player->hasSkill(this) && player->getPhase() == Player::Start) {
          player->setGender(player->getGeneral()->getGender());
          EXliyunpengImageChanged("EXliyunpeng", player, room);
      }
      else if (triggerEvent == EventPhaseChanging && player->hasSkill(this) && data.value<PhaseChangeStruct>().to == Player::NotActive) {
          player->setGender(General::Female);
          EXliyunpengImageChanged("EXliyunpeng_female", player, room);
      }
      else if (triggerEvent == GameStart && player->hasSkill(this)) {
          player->setGender(General::Female);
          EXliyunpengImageChanged("EXliyunpeng_female", player, room);
      }
      else if (triggerEvent == EventLoseSkill && data.toString() == objectName()) {
          player->setGender(player->getGeneral()->getGender());
          EXliyunpengImageChanged("EXliyunpeng", player, room);
      }
      else if (triggerEvent == EventAcquireSkill && data.toString() == objectName() && player->getPhase() == Player::NotActive) {
          player->setGender(General::Female);
          EXliyunpengImageChanged("EXliyunpeng_female", player, room);
      }
        return false;
    }
};

LienvCard::LienvCard() {
}
bool LienvCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const {
    return targets.isEmpty() && to_select->isWounded();
}
bool LienvCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const {
    return targets.value(0, Self)->isWounded();
}
void LienvCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const {
    room->cardEffect(this, source, targets.value(0, source));
}
void LienvCard::onEffect(const CardEffectStruct &effect) const {
    effect.to->getRoom()->recover(effect.to, RecoverStruct(effect.from));
}
class LienvViewAsSkill : public OneCardViewAsSkill {
public:
    LienvViewAsSkill() : OneCardViewAsSkill("lienv") {
        response_pattern = "@@lienv";
    }
    bool viewFilter(const Card *) const {
        return true;
    }
    const Card *viewAs(const Card *originalCard) const {
        LienvCard *lienvCard = new LienvCard;
        lienvCard->addSubcard(originalCard);
        return lienvCard;
    }
};
class Lienv : public TriggerSkill {
public:
    Lienv() : TriggerSkill("lienv") {
        events << Damage << Damaged << FinishJudge;
        view_as_skill = new LienvViewAsSkill;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = NULL;
        bool flag = false;
        if (triggerEvent == Damage) {
            target = damage.to;
            flag = player->getGender() == target->getGender();
        } else if (triggerEvent == Damaged) {
            target = damage.from;
            flag = player->getGender() != target->getGender();
        } else if (triggerEvent == FinishJudge) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == objectName()) {
                const Card *card = judge->card;
                if (card->isBlack()) {
                    player->obtainCard(card);
                } else {
                    if (!(player->isNude())) {
                        room->askForUseCard(player, "@@lienv", "@lienv_prompt");
                    }
                }
            }
            return false;
        }
        if (flag && room->askForSkillInvoke(player, objectName())) {
            JudgeStruct judge;
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;
            room->judge(judge);
        }
        return false;
    }
};

YigeCard::YigeCard() {}
bool YigeCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.length() == 0 && to_select->isFemale() && to_select != Self;
}
void YigeCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer* target = effect.to;
    ServerPlayer* player = effect.from;
    Room *room = player->getRoom();
    QStringList skill_names;
    QString skill_name;
    QString yige_skill = player->tag["yige_skill"].toString();

    if (yige_skill.length() > 0 && !(player->hasInnateSkill(yige_skill))) {
        room->handleAcquireDetachSkills(player, "-"+yige_skill, true);
    }
    player->tag.remove("yige_skill");
    foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
        if(p->getMark("@hongyanyige") > 0) {
            p->loseMark("@hongyanyige");
            if (p->hasSkill("hongyan")) {
                room->handleAcquireDetachSkills(p, "-hongyan", true);
            }
            break;
        }
    }

    foreach(const Skill * skill, target->getVisibleSkillList()) {
        //EX怀贝贝和左慈一样，不能复制主公技，限定技，觉醒技
        //这个框架贯石斧实现存在一定问题，一名角色装备贯石斧然后卸载后，会在技能列表里多出一个“贯石斧”的技能
        //黄天送牌，制霸拼点，陷嗣出杀，这些本不属于技能的范畴，但这个框架用技能来实现，这里也要屏蔽掉
        if (skill->isLordSkill() || skill->getFrequency() == Skill::Limited || skill->getFrequency() == Skill::Wake
                || skill->objectName() == "huangtian_attach"
                || skill->objectName() == "zhiba_pindian"
                || skill->objectName() == "xiansi_slash") {
            //|| skill->objectName() == "axe"
            continue;
        }
        if (!(skill_names.contains(skill->objectName()))) {
            skill_names.append(skill->objectName());
        }
    }
    skill_names.append("cancel");
    if(skill_names.length() > 0) {
        skill_name = room->askForChoice(player, "yige", skill_names.join("+"));
    }

    if (skill_name != "cancel") {
        if(!(player->hasSkill(skill_name))) {
            player->tag["yige_skill"] = QVariant(skill_name);
            room->handleAcquireDetachSkills(player, skill_name);
        }
        if (!(target->hasSkill("hongyan"))) {
            QString give_hongyan = room->askForChoice(player, "@give_hongyan", "yes+no");
            if (give_hongyan == "yes") {
                room->handleAcquireDetachSkills(target, "hongyan");
                target->gainMark("@hongyanyige");
            }
        }
    }
}
class YigeViewAsSkill : public ZeroCardViewAsSkill {
public:
    YigeViewAsSkill() : ZeroCardViewAsSkill("yige") {
        response_pattern = "@@yige";
    }
    const Card *viewAs() const {
        return new YigeCard;
    }
};
class Yige : public TriggerSkill{
public:
    Yige() : TriggerSkill("yige") {\
        events << GameStart << EventPhaseStart;
        view_as_skill = new YigeViewAsSkill;
    }
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        bool existFemale = false;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            //EX李云鹏的“蓝颜”实现不佳，回合开始时才修改性别，与此技能的时机冲突
            //EX李云鹏应该视为女性角色，这里做出弥补
            if (p->isFemale() || p->hasSkill("lanyan")) {
                existFemale = true;
                break;
            }
        }
        if (existFemale && triggerEvent == EventPhaseStart && player->getPhase() == Player::Start) {
            room->askForUseCard(player, "@@yige", "@yige");
        }
        else if (triggerEvent == GameStart && !existFemale) {
            QString choice = room->askForChoice(player, objectName(),"#yige_convert+cancel");
            if (choice == "#yige_convert") {
                room->notifySkillInvoked(player, objectName());
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                room->handleAcquireDetachSkills(player, "-yige|-jianmei|tiancheng");
            }
        }
        return false;
    }
};

class Jianmei : public TriggerSkill {
public:
    Jianmei() : TriggerSkill("jianmei$") {
        events << CardResponded << CardUsed << FinishJudge << EventPhaseChanging;
    }
    bool triggerable(const ServerPlayer *target) const {
        return target != NULL && target->hasSkill("hongyan");
    }
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        const Card *card = NULL;
        if (triggerEvent == CardResponded) {
            card = data.value<CardResponseStruct>().m_card;
        } else if (triggerEvent == CardUsed) {
            card = data.value<CardUseStruct>().card;
        } else if (triggerEvent == FinishJudge) {
            card = data.value<JudgeStruct *>()->card;
        } else if (triggerEvent == EventPhaseChanging && player->hasFlag("jianmeiUsed") && data.value<PhaseChangeStruct>().to == Player::NotActive) {
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer * p, players) {
                if(p->hasFlag("jianmeiInvoked")) {
                    room->setPlayerFlag(p, "-jianmeiInvoked");
                }
            }
        }
        bool can_invoke = false;
        if(card != NULL) {
            if (card->getSkillName() == "hongyan") {
                can_invoke = true;
            } else if(!(card->isKindOf("SkillCard")) && card->subcardsLength() > 0) {
                QList<int> subcards = card->getSubcards();
                foreach(int id, subcards) {
                    Card *subcard = Sanguosha->getCard(id);
                    if (subcard->getSkillName() == "hongyan") {
                        can_invoke = true;
                        break;
                    }
                }
            }
        }
        if (can_invoke) {
            QList<ServerPlayer *> beibis;
            foreach(ServerPlayer * p, room->getOtherPlayers(player)) {
                if(p->hasLordSkill(objectName()) && !(p->hasFlag("jianmeiInvoked"))) {
                    beibis.append(p);
                }
            }
            while (!(beibis.empty())) {
                ServerPlayer *beibi = room->askForPlayerChosen(player, beibis, objectName(), "@jianmei-to", true);
                if (beibi != NULL) {
                    if (!(beibi->isLord()) && beibi->hasSkill("weidi")) {
                        room->broadcastSkillInvoke("weidi");
                    } else {
                        room->broadcastSkillInvoke(objectName());
                    }

                    room->notifySkillInvoked(beibi, objectName());
                    LogMessage log;
                    log.type = "#InvokeOthersSkill";
                    log.from = player;
                    log.to << beibi;
                    log.arg = objectName();
                    room->sendLog(log);

                    beibi->drawCards(1, objectName());
                    room->setPlayerFlag(player, "jianmeiUsed");
                    room->setPlayerFlag(beibi, "jianmeiInvoked");
                    beibis.removeOne(beibi);
                }
                else {
                    break;
                }
            }
        }
        return false;
    }
};

DuanyanCard::DuanyanCard() {
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}
bool DuanyanCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *) const {
    return to_select->getPhase() != Player::NotActive;
}
void DuanyanCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer* target = effect.to;
    ServerPlayer* player = effect.from;
    Room *room = player->getRoom();
    target->obtainCard(this);
    room->damage(DamageStruct("duanyan", player, target, 1));
    int x = target->distanceTo(player) / 2;
    if(x > 0) {
        room->drawCards(target, x, "duanyan");
    }
}
class DuanyanViewAsSkill : public OneCardViewAsSkill {
public:
    DuanyanViewAsSkill() : OneCardViewAsSkill("duanyan") {
        filter_pattern = ".|diamond";
        response_pattern = "@@duanyan";
    }
    const Card *viewAs(const Card *originalCard) const {
        DuanyanCard *duanyanCard = new DuanyanCard;
        duanyanCard->addSubcard(originalCard);
        return duanyanCard;
    }
};

class Duanyan : public TriggerSkill {
public:
    Duanyan() : TriggerSkill("duanyan") {
        events << EventPhaseStart;
        view_as_skill = new DuanyanViewAsSkill;
    }
    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }
    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        if(player->getPhase() == Player::Start && player->isMale()) {
            QList<ServerPlayer *> jingmeizis = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *jingmeizi, jingmeizis) {
                if (jingmeizi->canDiscard(jingmeizi, "he")) {
                    room->askForUseCard(jingmeizi, "@@duanyan", "@duanyan-prompt");
                }
            }
        }
        return false;
    }
};

class Pingfeng : public TriggerSkill {
public:
    Pingfeng() : TriggerSkill("pingfeng") {
        events << CardsMoveOneTime << EventAcquireSkill << EventLoseSkill << GameStart;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == EventLoseSkill && data.toString() == objectName()) {
            room->handleAcquireDetachSkills(player, "-feiying|-liuli", true);
        } else if (triggerEvent == EventAcquireSkill && data.toString() == objectName()) {
            room->notifySkillInvoked(player, objectName());
            if (player->hasEquip()) {
                room->handleAcquireDetachSkills(player, "liuli");
            }
            else {
                room->handleAcquireDetachSkills(player, "feiying");
            }
        } else if (triggerEvent == CardsMoveOneTime && player->isAlive() && player->hasSkill(this, true)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to && move.to == player && move.to_place == Player::PlaceEquip) {
                if (player->getEquips().length() == 1) {
                    room->handleAcquireDetachSkills(player,"-feiying|liuli", true);
                }
            } else if (move.from && move.from == player && move.from_places.contains(Player::PlaceEquip)) {
                if (!(player->hasEquip())) {
                    room->handleAcquireDetachSkills(player,"feiying|-liuli", true);
                }
            }
        } else if(triggerEvent == GameStart && player->hasSkill(this, true)) {
            room->handleAcquireDetachSkills(player, "feiying");
        }
        return false;
    }
};

YunEXPackage::YunEXPackage()
    :Package("yunEX")
{
    General *EXliyunpeng = new General(this, "EXliyunpeng", "wu", 3, true); // YunEX 001
    General *EXliyunpeng_female = new General(this, "EXliyunpeng_female", "wu", 3, false, true, true); // YunEX 001
    EXliyunpeng->addSkill(new Lanyan);
    EXliyunpeng->addSkill(new Lienv);
    EXliyunpeng_female->addSkill("lanyan");
    EXliyunpeng_female->addSkill("lienv");

    General *EXhuaibeibei = new General(this, "EXhuaibeibei$", "wu", 4, false); // YunEX 002
    EXhuaibeibei->addSkill("hongyan");
    EXhuaibeibei->addSkill(new Yige);
    EXhuaibeibei->addSkill(new Jianmei);

    General *EXhanjing = new General(this, "EXhanjing", "wu", 3, false); // YunEX 003
    EXhanjing->addSkill(new Duanyan);
    EXhanjing->addSkill(new Pingfeng);

    addMetaObject<LienvCard>();
    addMetaObject<YigeCard>();
    addMetaObject<DuanyanCard>();
}

ADD_PACKAGE(YunEX)
