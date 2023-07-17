#include "settings.h"
#include "standard.h"
#include "maneuvering.h"
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
#include "json.h"
#include "protocol.h"

bool askForForkSkillInvoke(Room *room, ServerPlayer *player, const QString &skill_name, const QString &fork_name, const QVariant &data) {
    bool flag = player->askForSkillInvoke(skill_name+fork_name, data);
    if(flag) {
        room->broadcastSkillInvoke(skill_name);
        room->notifySkillInvoked(player, skill_name);
        LogMessage log;
        log.type = "#InvokeSkill";
        log.from = player;
        log.arg = skill_name+fork_name;
        room->sendLog(log);
    }
    return flag;
}

LureTiger::LureTiger(Card::Suit suit, int number)
    : TrickCard(suit, number) {
    setObjectName("lure_tiger");
}

QString LureTiger::getSubtype() const {
    return "lure_tiger";
}

bool LureTiger::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    int total_num = 2 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;
    if (Self->isCardLimited(this, Card::MethodUse))
        return false;

    return to_select != Self;
}

void LureTiger::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const {
    QStringList nullified_list = room->getTag("CardUseNullifiedList").toStringList();
    bool all_nullified = nullified_list.contains("_ALL_TARGETS");
    foreach (ServerPlayer *target, targets) {
        CardEffectStruct effect;
        effect.card = this;
        effect.from = source;
        effect.to = target;
        effect.multiple = (targets.length() > 1);
        effect.nullified = (all_nullified || nullified_list.contains(target->objectName()));

        QVariantList players;
        for (int i = targets.indexOf(target); i < targets.length(); i++) {
            if (!nullified_list.contains(targets.at(i)->objectName()) && !all_nullified)
                players.append(QVariant::fromValue(targets.at(i)));
        }
        room->setTag("targets" + this->toString(), QVariant::fromValue(players));

        room->cardEffect(effect);
    }

    room->removeTag("targets" + this->toString());

    /*QList<int> table_cardids = room->getCardIdsOnTable(this);
    if (!table_cardids.isEmpty()) {
        DummyCard dummy(table_cardids);
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName(), QString(), this->getSkillName(), QString());
        if (targets.size() == 1) reason.m_targetId = targets.first()->objectName();
        room->moveCardTo(&dummy, source, NULL, Player::DiscardPile, reason, true);
    }*/
}

void LureTiger::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.to->getRoom();

    room->setPlayerCardLimitation(effect.to, "use", ".", false);
    room->setPlayerProperty(effect.to, "removed", true);
    effect.from->setFlags("LureTigerUser");
}

class LureTigerSkill : public TriggerSkill
{
public:
    LureTigerSkill() : TriggerSkill("lure_tiger-effect") {
        events << Death << EventPhaseChanging;
        global = true;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (!player->hasFlag("LureTigerUser"))
            return false;
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        } else if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
        }

        foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->isRemoved()) {
                room->setPlayerProperty(p, "removed", false);
                room->removePlayerCardLimitation(p, "use", ".$0");
            }
        }

        return false;
    }
};

class LureTigerProhibit : public ProhibitSkill {
public:
    LureTigerProhibit() : ProhibitSkill("#lure_tiger-prohibit") {}

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const
    {
        return to->isRemoved() && card->getTypeId() != Card::TypeSkill;
    }
};

class LureTigerLockHPSkill : public TriggerSkill {
public:
    LureTigerLockHPSkill() : TriggerSkill("#lure_tiger-lockHP") {
        events << PreHpLost << PreHpRecover << Predamage;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(triggerEvent == PreHpLost) {
            if(player->isRemoved()) {
                return true;
            }
        }else if(triggerEvent == PreHpRecover) {
            if(player->isRemoved()) {
                return true;
            }
        }else if(triggerEvent == Predamage) {
            if(data.value<DamageStruct>().to->isRemoved()) {
                return true;
            }
        }
        return false;
    }

};

class Tiancheng : public TriggerSkill {
public:
     Tiancheng() : TriggerSkill("tiancheng") {
         events << CardResponded << CardUsed << FinishJudge << EventPhaseChanging;
         frequency = Frequent;
     }
     bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
     {
        const Card *card = NULL;
        const QString tianchengkeepMark = "@hongyantiancheng";
        const QString tianchengTriggeredFlag = "tianchengTriggered";

        if (triggerEvent == CardResponded) {
            card = data.value<CardResponseStruct>().m_card;
        } else if(triggerEvent == CardUsed) {
            card = data.value<CardUseStruct>().card;
        } else if(triggerEvent == FinishJudge) {
            card = data.value<JudgeStruct *>()->card;
        } else if(triggerEvent == EventPhaseChanging) {
            if(data.value<PhaseChangeStruct>().to == Player::NotActive) {
                 player->loseAllMarks(tianchengkeepMark);
                 return false;
            }
        }

        int count = 0;
        if(card) {
            if(card->getSkillName() == "hongyan") {
                count++;
            }else if (!card->isVirtualCard() && card->subcardsLength() > 0)  {
                QList<int> subcards = card->getSubcards();
                foreach (int cardID, subcards) {
                    if(Sanguosha->getCard(cardID)->getSkillName() == "hongyan") {
                        count++;
                        break;
                    }
                }
            }
        }

        if(count > 0) {
            if(player->askForSkillInvoke(this)) {
                player->drawCards(1, objectName());
                if(player->getPhase() == Player::Play) {
                    if(player->hasFlag(tianchengTriggeredFlag)) {
                        player->gainMark(tianchengkeepMark);
                        room->askForDiscard(player, objectName(), 1, 1, false, true);
                    } else {
                        player->setFlags(tianchengTriggeredFlag);
                    }
                }
            }
        }
        return false;
     }
};

class tianchengkeep : public MaxCardsSkill {
public:
    tianchengkeep() : MaxCardsSkill("#tianchengkeep") {
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
            if (player->hasSkill(this) && player->isAlive() && player->getPhase() == Player::NotActive && player->canSlash(victim)) {
                jingmeizis.append(player);
            }
        }
        foreach(ServerPlayer *jingmeizi, jingmeizis) {
            if (room->askForUseSlashTo(jingmeizi, victim, "@lianji-prompt")) {
                room->notifySkillInvoked(jingmeizi, objectName());
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = jingmeizi;
                log.arg = objectName();
                room->sendLog(log);
                jingmeizi->drawCards(1, objectName());
            }
        }
        return false;
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
        events << Damaged << CardUsed << CardResponded;
        frequency = Frequent;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(triggerEvent == Damaged) {
            for(int i = 0; i < data.value<DamageStruct>().damage; i++) {
                if(player->isAlive() && player->askForSkillInvoke(this, data)) {
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
        else if(triggerEvent == CardUsed || triggerEvent == CardResponded) {
            bool flag = false;
            if(triggerEvent == CardUsed) {
                CardUseStruct use = data.value<CardUseStruct>();
                flag = !use.m_isHandcard && !use.card->isKindOf("SkillCard");
            }else if(triggerEvent == CardResponded) {
                CardResponseStruct res = data.value<CardResponseStruct>();
                flag = !res.m_isHandcard && !res.m_card->isKindOf("SkillCard");
            }

            if(flag && askForForkSkillInvoke(room, player, objectName(), "-draw", data)) {
                player->drawCards(1, objectName());
            }
        }
        return false;
    }
};

class SiwuLA : public TriggerSkill
{
public:
    SiwuLA() : TriggerSkill("#siwula") {
        events << EventLoseSkill << EventAcquireSkill;
    }

    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(triggerEvent == EventLoseSkill && data.toString() == "siwu") {
            player->pileToPile("&wire", "wire", false);
        } else if(triggerEvent == EventAcquireSkill && data.toString() == "siwu") {
            player->pileToPile("wire", "&wire", false);
        }
        return false;
    }
};

class Xingcan : public TriggerSkill
{
public:
    Xingcan() : TriggerSkill("xingcan") {
        events << EventPhaseEnd;
        frequency = NotFrequent;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == EventPhaseEnd && player->getPhase() == Player::Play) {
            const Card* card = NULL;
            card = room->askForCard(player, ".", "@xingcan-put", NULL, Card::MethodNone);
            if(card != NULL) {
                CardMoveReason reason(CardMoveReason::S_REASON_PUT, player->objectName(), objectName(), QString());
                CardsMoveStruct move(card->getEffectiveId(), player, NULL, Player::PlaceHand, Player::DrawPile, reason);
                room->moveCardsAtomic(move, false);

                player->skip(Player::Discard);
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
        if(player->getPhase() == Player::Play && data.value<CardUseStruct>().card->isKindOf("Slash") && !player->hasFlag("zhanguiSlashUsed")) {
            room->setPlayerFlag(player, "zhanguiSlashUsed");
        }
        return false;
    }
};

class DiaolueVS : public OneCardViewAsSkill {
public:
    DiaolueVS() : OneCardViewAsSkill("diaolue") {
        filter_pattern = ".|red|.|hand";
        response_or_use = true;
    }
    const Card *viewAs(const Card *originalCard) const {
        LureTiger *lureTiger = new LureTiger(originalCard->getSuit(), originalCard->getNumber());
        lureTiger->addSubcard(originalCard->getId());
        lureTiger->setSkillName(objectName());
        return lureTiger;
    }
};

class Diaolue : public TriggerSkill {
public:
    Diaolue() : TriggerSkill("diaolue") {
        events << CardFinished << EventPhaseChanging << TrickEffect;
        view_as_skill = new DiaolueVS;
    }

    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const {
        if(event == CardFinished && player->hasSkill(this)) {
            if(data.value<CardUseStruct>().card->objectName()=="lure_tiger" && askForForkSkillInvoke(room,player,objectName(),"-draw",data)) {
                player->drawCards(1, objectName());
            }
        }else if(event == TrickEffect) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            const Card* card = effect.card;
            ServerPlayer *from = effect.from;
            if(card->objectName() == "lure_tiger" && from->hasSkill(this)) {
                player->gainMark("@diaohulishan");
            }
        } else if(event == EventPhaseChanging && player->hasSkill(this)) {
            if(data.value<PhaseChangeStruct>().to == Player::NotActive) {
                foreach(ServerPlayer* p, room->getOtherPlayers(player)) {
                    p->loseMark("@diaohulishan");
                }
            }
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
            && askForForkSkillInvoke(room,player,objectName(),"-ice_sword",data)) {
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
            player->getRoom()->askForUseCard(player, "@@miyu", QString("@miyu:::%1").arg(Self->getLostHp() / 2 + Self->getLostHp() % 2));

        }
        return false;
    }
};

class YingzhouViewAsSkill : public OneCardViewAsSkill {
public:
    YingzhouViewAsSkill() : OneCardViewAsSkill("yingzhou") {
        response_or_use = true;
    }
    bool isEnabledAtPlay(const Player *player) const {
        if(player->hasFlag("yingzhouEffect")) {
            //这里只好用这种方式进行检测，C++没有Java的反射机制
            //这里只检测【杀】，【酒】
            //这里就算写得有问题，也不会导致逻辑错误。顶多是显示缺陷，即按钮亮着，牌无法使用。
            if(player->property("yingzhouCard").toString().contains("Slash")) return Slash::IsAvailable(player);
            else if(player->property("yingzhouCard").toString().contains("Analeptic")) return Analeptic::IsAvailable(player);
            else return true;
        }
        return false;
    }
    bool isEnabledAtResponse(const Player *player, const QString &pattern) const {
        if(player->getPhase() == Player::Play) {
            return pattern.contains(player->property("yingzhouCard").toString().toLower());
        }
        return false;
    }
    bool isEnabledAtNullification(const ServerPlayer *player) const {
        return player->getPhase() == Player::Play && player->property("yingzhouCard").toString() == "Nullification";
    }
    bool viewFilter(const Card *card) const {
        if (card->isEquipped())
            return false;

        return card->getColor() != Self->property("yingzhouColor").toInt();

    }
    const Card *viewAs(const Card *originalCard) const {
        Card* acard = Sanguosha->cloneCard(Self->property("yingzhouCard").toString(), originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName(objectName());
        acard->setCanRecast(false);
        return acard;
    }
};

class Yingzhou : public TriggerSkill {
public:
    Yingzhou() : TriggerSkill("yingzhou")
    {
        events << CardUsed;
        view_as_skill = new YingzhouViewAsSkill;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(player->getPhase() == Player::Play) {
            const Card *card = data.value<CardUseStruct>().card;
            if(card) {
                if(card->getSkillName() == objectName()) {
                    room->setPlayerFlag(player, "-yingzhouEffect");
                }else {
                    if(card->isNDTrick() || card->isKindOf("BasicCard")) {
                        room->setPlayerProperty(player, "yingzhouCard", QVariant(card->getClassName()));
                        room->setPlayerProperty(player, "yingzhouColor",QVariant(card->getColor()));
                        room->setPlayerFlag(player, "yingzhouEffect");
                    }else if(card->isKindOf("SkillCard")) {}
                    else room->setPlayerFlag(player, "-yingzhouEffect");
                }
            }
        }
        return false;
    }
};

QifengCard::QifengCard() {

}
bool QifengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.length() < 1 && !to_select->hasFlag("qifeng_original");
}
bool QifengCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const {

}
void QifengCard::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.from->getRoom();
    DamageStruct damage = room->getTag("QifengDamage").value<DamageStruct>();

    /*bool invoke_transfer = false;
    if (target->canDiscard(target, "h")) {
        if (!(room->askForDiscard(target, "qifeng", 1, 1, true, false, "@qifeng-discard", ".|heart|.|hand"))) {
            invoke_transfer = true;
        }
    }
    else invoke_transfer = true;*/
    //if (invoke_transfer) {
    DamageStruct damage2(damage.reason, damage.from, effect.to, damage.damage, damage.nature);
    damage2.transfer = true;
    damage2.transfer_reason = "qifeng";

    //damage.damage = 0;
    //data.setValue(damage);
    //room->setTag("qifengdamage", data);

    room->damage(damage2);
    //}
    /*DamageStruct damage = effect.from->tag.value("QifengDamage").value<DamageStruct>();
    ServerPlayer* ori = damage.to;
    damage.to = effect.to;
    damage.transfer = true;
    damage.transfer_reason = "qifeng";
    ori->tag["TransferDamage"] = QVariant::fromValue(damage);*/
}
class QifengViewAsSkill : public OneCardViewAsSkill {
public:
    QifengViewAsSkill() : OneCardViewAsSkill("qifeng") {
        filter_pattern = ".|spade|.|hand!";
        response_pattern = "@@qifeng";
    }
    const Card *viewAs(const Card *originalCard) const {
        QifengCard *card = new QifengCard;
        card->addSubcard(originalCard);
        return card;
    }
};
class Qifeng : public TriggerSkill {
public:
    Qifeng() : TriggerSkill("qifeng") {
        events << DamageCaused;
        view_as_skill = new QifengViewAsSkill;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        //const Card* res = room->askForUseCard(player, "Jink", "@qifeng-card", -1, Card::MethodDiscard);
        //const Card* res = room->askForUseCard(player, "@@qifeng", "@qifeng-card", -1, Card::MethodDiscard);
        DamageStruct damage = data.value<DamageStruct>();
        const Card* res = NULL;
        if (damage.to != player && !(damage.transfer == true && damage.transfer_reason == objectName())) {
            //QList<ServerPlayer *> players = room->getOtherPlayers(player);
            //players.removeOne(target);
            room->setTag("QifengDamage", data);
            room->setPlayerFlag(damage.to, "qifeng_original");
            //if (room->askForUseCard(player, "@@qifeng", "@qifeng", -1, Card::MethodDiscard)) {
                //if (room->getTag("QifengDamage").value<DamageStruct>().damage == 0) {
                //transfer = true;
                //}
            //}
            res = room->askForUseCard(player, "@@qifeng", "@qifeng-card", -1, Card::MethodDiscard);
            room->setPlayerFlag(damage.to, "-qifeng_original");
        }
        return res;
    }
};
class QifengDraw : public TriggerSkill
{
public:
    QifengDraw() : TriggerSkill("#qifengdraw")
    {
        events << DamageComplete;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (player->isAlive() && damage.transfer && damage.transfer_reason == "qifeng")
            player->drawCards(player->getLostHp(), objectName());
        return false;
    }
};

YunPackage::YunPackage()
    :Package("yun")
{
    General *huaibeibei = new General(this, "huaibeibei", "wu", 4, false); // Yun 006
    huaibeibei->addSkill("hongyan");
    huaibeibei->addSkill(new Tiancheng);
    huaibeibei->addSkill(new tianchengkeep);
    related_skills.insertMulti("tiancheng", "#tianchengkeep");

    General *hanjing = new General(this, "hanjing", "wu", 3, false); // Yun 007
    hanjing->addSkill(new Lianji);
    hanjing->addSkill(new Qiaopo);

    General *wangcan = new General(this, "wangcan", "wei", 3, false); // Yun 008
    wangcan->addSkill(new Siwu);
    wangcan->addSkill(new SiwuLA);
    related_skills.insertMulti("siwu", "#siwula");
    wangcan->addSkill(new Xingcan);

    General *yangwenqi = new General(this, "yangwenqi", "shu", 4, false); // Yun 009
    yangwenqi->addSkill(new Zhangui);
    yangwenqi->addSkill(new Diaolue);

    General *xiaosa = new General(this, "xiaosa", "wei", 4, false); // Yun 010
    xiaosa->addSkill(new Xiaohan);
    xiaosa->addSkill(new Miyu);

    General *lishuyu = new General(this, "lishuyu", "shu", 3, false); // Yun 011
    lishuyu->addSkill(new Yingzhou);
    lishuyu->addSkill(new Qifeng);
    lishuyu->addSkill(new QifengDraw);
    related_skills.insertMulti("qifeng", "#qifengdraw");

    addMetaObject<QiaopoCard>();
    addMetaObject<MiyuCard>();
    addMetaObject<QifengCard>();

    QList<Card *> cards;
    cards << new LureTiger(Card::Heart, 2);

    skills << new LureTigerProhibit << new LureTigerLockHPSkill << new LureTigerSkill;
    insertRelatedSkills("lure_tiger-effect", "#lure_tiger-prohibit");
    insertRelatedSkills("lure_tiger-effect", "#lure_tiger-lockHP");

    cards.first()->setParent(this);
}

ADD_PACKAGE(Yun)

static const QString lanyanName("lanyan");
class Lanyan : public TriggerSkill{
public:
    Lanyan() : TriggerSkill(lanyanName) {
        events << GameStart << TurnStart << EventPhaseChanging << EventAcquireSkill << EventLoseSkill;
        frequency = Compulsory;
    }
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if(triggerEvent == TurnStart) {
            changeEXliyunpengGender(player, room, false);
        }
        else if (triggerEvent == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::NotActive) {
            changeEXliyunpengGender(player, room, true);
        }
        else if (triggerEvent == GameStart) {
            changeEXliyunpengGender(player, room, true);
        }
        return false;
    }
    static void changeEXliyunpengGender(ServerPlayer* player, Room* room, bool lanyan_female) {
        if(lanyan_female) {
            room->notifySkillInvoked(player, lanyanName);
            LogMessage log;
            log.type = "#lanyan";
            log.from = player;
            log.arg = lanyanName;
            log.arg2 = "female";
            room->sendLog(log);

            //这个Mark用来标志男/女插画切换
            room->setPlayerMark(player, "lanyan", 1);
            player->setGender(General::Female);
        } else {
            room->setPlayerMark(player, "lanyan", 0);
            player->setGender(player->getGeneral()->getGender());
        }
    }
};

class LanyanLA : public TriggerSkill
{
public:
    LanyanLA() : TriggerSkill("#lanyanla") {
        events << EventLoseSkill << EventAcquireSkill;
    }

    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == EventLoseSkill && data.toString() == lanyanName) {
            Lanyan::changeEXliyunpengGender(player, room, false);
        }
        else if (triggerEvent == EventAcquireSkill && data.toString() == lanyanName && player->getPhase() == Player::NotActive) {
            Lanyan::changeEXliyunpengGender(player, room, true);
        }
        return false;
    }
};

LienvCard::LienvCard() {}
bool LienvCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.isEmpty() && to_select->isWounded() && Self->getGender()==to_select->getGender();
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
        } else if (triggerEvent == Damaged) {
            target = damage.from;
        } else if (triggerEvent == FinishJudge) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == objectName()) {
                const Card *card = judge->card;
                if (card->isBlack()) {
                    player->obtainCard(card);
                } else {
                    if (!(player->isNude())) {
                        QString gender="";
                        if(player->isMale())gender="male";
                        else if(player->isFemale())gender="female";
                        room->askForUseCard(player, "@@lienv", QString("@lienv-prompt:::%1").arg(gender));
                    }
                }
            }
            return false;
        }
        if (player->askForSkillInvoke(this, data)) {
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
        //黄天送牌，制霸拼点，陷嗣出杀，这些本不属于技能的范畴，但这个框架用技能来实现，这里也要屏蔽掉
        if (skill->isLordSkill() || skill->getFrequency() == Skill::Limited || skill->getFrequency() == Skill::Wake
                || skill->objectName() == "huangtian_attach"
                || skill->objectName() == "zhiba_pindian"
                || skill->objectName() == "xiansi_slash") {
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
            if (p->isFemale()) {
                existFemale = true;
                break;
            }
        }
        if (existFemale && triggerEvent == EventPhaseStart && player->getPhase() == Player::Start) {
            room->askForUseCard(player, "@@yige", "@yige");
        }
        else if (triggerEvent == GameStart) {
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

    if(effect.to->isMale()) {
        int x = target->distanceTo(player) / 2;
        if(x > 0) {
            room->drawCards(target, x, "duanyan");
        }
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
        if(player->getPhase() == Player::Play) {
            QList<ServerPlayer *> jingmeizis = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *jingmeizi, jingmeizis) {
                if (jingmeizi != player && jingmeizi->canDiscard(jingmeizi, "he")) {
                    QString prompt;
                    if(player->isMale()) prompt = QString("@duanyan-prompt-male:::%1:%2").arg(player->getGeneralName()).arg(player->distanceTo(jingmeizi) / 2);
                    else prompt = QString("@duanyan-prompt:::%1").arg(player->getGeneralName());
                    room->askForUseCard(jingmeizi, "@@duanyan", prompt);
                }
            }
        }
        return false;
    }
};

class Pingfeng : public TriggerSkill {
public:
    Pingfeng() : TriggerSkill("pingfeng") {
        events << CardsMoveOneTime << GameStart;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == CardsMoveOneTime && player->isAlive()) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to && move.to == player && move.to_place == Player::PlaceEquip) {
                if (player->getEquips().length() == 1) {
                    room->notifySkillInvoked(player, objectName());
                    room->handleAcquireDetachSkills(player,"-feiying|liuli", true);
                }
            } else if (move.from && move.from == player && move.from_places.contains(Player::PlaceEquip)) {
                if (!(player->hasEquip())) {
                    room->notifySkillInvoked(player, objectName());
                    room->handleAcquireDetachSkills(player,"feiying|-liuli", true);
                }
            }
        } else if(triggerEvent == GameStart) {
            room->notifySkillInvoked(player, objectName());
            if (player->hasEquip()) {
                room->handleAcquireDetachSkills(player, "liuli");
            }
            else {
                room->handleAcquireDetachSkills(player, "feiying");
            }
        }
        return false;
    }
};
class PingfengLA : public TriggerSkill
{
public:
    PingfengLA() : TriggerSkill("#pingfengla") {
        events << EventLoseSkill << EventAcquireSkill;
    }

    bool triggerable(const ServerPlayer *target) const {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == EventLoseSkill && data.toString() == "pingfeng") {
            room->handleAcquireDetachSkills(player, "-feiying|-liuli", true);
        } else if (triggerEvent == EventAcquireSkill && data.toString() == "pingfeng") {
            room->notifySkillInvoked(player, objectName());
            if (player->hasEquip()) {
                room->handleAcquireDetachSkills(player, "liuli");
            }
            else {
                room->handleAcquireDetachSkills(player, "feiying");
            }
        }
        return false;
    }
};

class LeiyaViewAsSkill : public ZeroCardViewAsSkill {
public:
    LeiyaViewAsSkill() : ZeroCardViewAsSkill("leiya") {
        response_pattern = "@@leiya";
    }
    const Card *viewAs() const {
        IronChain* card = new IronChain(Card::NoSuitBlack, 0);
        card->setSkillName(objectName());
        return card;
    }
};

class Leiya : public TriggerSkill {
public:
    Leiya() : TriggerSkill("leiya") {
        events << Damage << CardFinished << Predamage ;
        view_as_skill = new LeiyaViewAsSkill;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        if (triggerEvent == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->isKindOf("Slash")) {
                room->setPlayerFlag(player, "leiya");
            }
        }else if(triggerEvent == CardFinished) {
            if(player->hasFlag("leiya") && data.value<CardUseStruct>().card->isKindOf("Slash")) {
                room->setPlayerFlag(player, "-leiya");
                room->askForUseCard(player, "@@leiya", "@leiya-card");
            }
        } else if(triggerEvent == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from == player && (damage.card->isKindOf("Slash") || (damage.card->isKindOf("Duel")))) {
                room->broadcastSkillInvoke(objectName());
                room->sendCompulsoryTriggerLog(player, objectName());
                damage.nature = DamageStruct::Thunder;
                data.setValue(damage);
            }
        }
        return false;
    }
};

class ZhenyueViewAsSkill : public ZeroCardViewAsSkill {
public:
    ZhenyueViewAsSkill() : ZeroCardViewAsSkill("zhenyue") {
        response_pattern = "@@zhenyue";
    }
    const Card *viewAs() const {
        ThunderSlash* card = new ThunderSlash(Card::NoSuit, 0);
        card->setSkillName(objectName());
        return card;
    }
};

class Zhenyue : public TriggerSkill
{
public:
    Zhenyue() : TriggerSkill("zhenyue")
    {
        events << EventPhaseChanging << CardUsed;
        view_as_skill = new ZhenyueViewAsSkill;

        frequency = Limited;
        limit_mark = "@zhenyue";
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if(player->getMark("@zhenyue")==0) return false;
        if (event == EventPhaseChanging && data.value<PhaseChangeStruct>().to == Player::Draw && !player->isSkipped(Player::Draw) && !player->isSkipped(Player::Play) && !player->isSkipped(Player::Discard)) {
            if (Slash::IsAvailable(player)) {
                int x = player->getLostHp() / 2 + player->getLostHp() % 2;

                if(room->askForUseCard(player, "@@zhenyue", QString("@zhenyue-slash:::%1").arg(x))) {
                    player->skip(Player::Draw, true);
                    player->skip(Player::Play, true);
                    player->skip(Player::Discard, true);
                }
            }
        }else if(event == CardUsed) {
            if(data.value<CardUseStruct>().card->getSkillName() == objectName()) {
                room->removePlayerMark(player, "@zhenyue");
                room->broadcastSkillInvoke(objectName());
                room->doSuperLightbox("EXxiaosa", objectName());
            }
        }
        return false;
    }
};

class ZhenyueTargetMod : public TargetModSkill
{
public:
    ZhenyueTargetMod() : TargetModSkill("#zhenyue-mod"){}

    int getDistanceLimit(const Player *from, const Card *card) const
    {
        if(card->getSkillName() == "zhenyue")
            return 1024;
        else
            return 0;
    }

    int getExtraTargetNum(const Player *from, const Card *card) const
    {
        if (card->getSkillName() == "zhenyue")
            return from->getLostHp() / 2 + from->getLostHp() % 2;
        else
            return 0;
    }
};

YunEXPackage::YunEXPackage()
    :Package("yunEX")
{
    General *EXliyunpeng = new General(this, "EXliyunpeng", "wu", 3, true); // YunEX 001
    EXliyunpeng->addSkill(new Lanyan);
    EXliyunpeng->addSkill(new LanyanLA);
    related_skills.insertMulti(lanyanName, "#lanyanla");
    EXliyunpeng->addSkill(new Lienv);

    General *EXhuaibeibei = new General(this, "EXhuaibeibei$", "wu", 4, false); // YunEX 002
    EXhuaibeibei->addSkill("hongyan");
    EXhuaibeibei->addSkill(new Yige);
    EXhuaibeibei->addSkill(new Jianmei);
    EXhuaibeibei->addRelateSkill("tiancheng");

    General *EXhanjing = new General(this, "EXhanjing", "wu", 3, false); // YunEX 003
    EXhanjing->addSkill(new Duanyan);
    EXhanjing->addSkill(new Pingfeng);
    EXhanjing->addSkill(new PingfengLA);
    EXhanjing->addRelateSkill("feiying");
    EXhanjing->addRelateSkill("liuli");
    related_skills.insertMulti("pingfeng", "#pingfengla");

    General *EXxiaosa = new General(this, "EXxiaosa", "wei", 4, false); // YunEX 004
    EXxiaosa->addSkill(new Leiya);
    EXxiaosa->addSkill(new Zhenyue);
    EXxiaosa->addSkill(new ZhenyueTargetMod);
    related_skills.insertMulti("zhenyue", "#zhenyue-mod");

    addMetaObject<LienvCard>();
    addMetaObject<YigeCard>();
    addMetaObject<DuanyanCard>();
}

ADD_PACKAGE(YunEX)

