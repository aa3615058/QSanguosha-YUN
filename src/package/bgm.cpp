#include "bgm.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "engine.h"
#include "settings.h"
#include "standard-skillcards.h"
#include "util.h"
#include "wrapped-card.h"
#include "room.h"
#include "roomthread.h"

class Chongzhen : public TriggerSkill
{
public:
    Chongzhen() : TriggerSkill("chongzhen")
    {
        events << CardResponded << TargetSpecified;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == CardResponded) {
            CardResponseStruct resp = data.value<CardResponseStruct>();
            if (resp.m_card->getSkillName() == "longdan"
                && resp.m_who != NULL && !resp.m_who->isKongcheng()) {
                QVariant data = QVariant::fromValue(resp.m_who);
                if (player->askForSkillInvoke(this, data)) {
                    room->broadcastSkillInvoke("chongzhen", 1);
                    int card_id = room->askForCardChosen(player, resp.m_who, "h", objectName());
                    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                    room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                }
            }
        } else {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->getSkillName() == "longdan") {
                foreach (ServerPlayer *p, use.to) {
                    if (p->isKongcheng()) continue;
                    QVariant data = QVariant::fromValue(p);
                    p->setFlags("ChongzhenTarget");
                    bool invoke = player->askForSkillInvoke(this, data);
                    p->setFlags("-ChongzhenTarget");
                    if (invoke) {
                        room->broadcastSkillInvoke("chongzhen", 2);
                        int card_id = room->askForCardChosen(player, p, "h", objectName());
                        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                        room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                    }
                }
            }
        }
        return false;
    }
};

LihunCard::LihunCard()
{
    mute = true;
}

bool LihunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select->isMale() && to_select != Self;
}

void LihunCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    effect.to->setFlags("LihunTarget");
    effect.from->setFlags("LihunSource");// for ai
    effect.from->turnOver();
    room->broadcastSkillInvoke("lihun", 1);
    DummyCard dummy_card(effect.to->handCards());

    try {
        if (!effect.to->isKongcheng()) {
            CardMoveReason reason(CardMoveReason::S_REASON_TRANSFER, effect.from->objectName(),
                effect.to->objectName(), "lihun", QString());
            room->moveCardTo(&dummy_card, effect.to, effect.from, Player::PlaceHand, reason, false);
        }
        effect.from->setFlags("-LihunSource");
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
            effect.from->setFlags("-LihunSource");
            effect.to->setFlags("-LihunTarget");
        }
        throw triggerEvent;
    }
}

class LihunSelect : public OneCardViewAsSkill
{
public:
    LihunSelect() : OneCardViewAsSkill("lihun")
    {
        filter_pattern = ".!";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->canDiscard(player, "he") && !player->hasUsed("LihunCard");
    }

    const Card *viewAs(const Card *originalCard) const
    {
        LihunCard *card = new LihunCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Lihun : public TriggerSkill
{
public:
    Lihun() : TriggerSkill("lihun")
    {
        events << EventPhaseStart << EventPhaseEnd;
        view_as_skill = new LihunSelect;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->hasUsed("LihunCard");
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *diaochan, QVariant &) const
    {
        if (triggerEvent == EventPhaseEnd && diaochan->getPhase() == Player::Play) {
            ServerPlayer *target = NULL;
            foreach (ServerPlayer *other, room->getOtherPlayers(diaochan)) {
                if (other->hasFlag("LihunTarget")) {
                    other->setFlags("-LihunTarget");
                    target = other;
                    break;
                }
            }

            if (!target || target->getHp() < 1 || diaochan->isNude())
                return false;

            room->broadcastSkillInvoke(objectName(), 2);
            DummyCard *to_goback;
            if (diaochan->getCardCount() <= target->getHp()) {
                to_goback = diaochan->isKongcheng() ? new DummyCard : diaochan->wholeHandCards();
                for (int i = 0; i < 5; i++)
                    if (diaochan->getEquip(i))
                        to_goback->addSubcard(diaochan->getEquip(i)->getEffectiveId());
            } else
                to_goback = (DummyCard *)room->askForExchange(diaochan, objectName(), target->getHp(), target->getHp(), true, "LihunGoBack");

            CardMoveReason reason(CardMoveReason::S_REASON_GIVE, diaochan->objectName(),
                target->objectName(), objectName(), QString());
            room->moveCardTo(to_goback, diaochan, target, Player::PlaceHand, reason);
            delete to_goback;
        } else if (triggerEvent == EventPhaseStart && diaochan->getPhase() == Player::NotActive) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("LihunTarget"))
                    p->setFlags("-LihunTarget");
            }
        }

        return false;
    }
};

class Kuiwei : public TriggerSkill
{
public:
    Kuiwei() : TriggerSkill("kuiwei")
    {
        events << EventPhaseStart;
    }

    static int getWeaponCount(ServerPlayer *caoren)
    {
        int n = 0;
        foreach (ServerPlayer *p, caoren->getRoom()->getAlivePlayers()) {
            if (p->getWeapon()) n++;
        }
        return n;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target && target->isAlive()
            && (target->hasSkill(this) || target->getMark("@kuiwei") > 0);
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *caoren, QVariant &) const
    {
        if (caoren->getPhase() == Player::Finish) {
            if (!caoren->hasSkill(this))
                return false;
            if (!caoren->askForSkillInvoke(this))
                return false;

            room->broadcastSkillInvoke(objectName());
            int n = getWeaponCount(caoren);
            caoren->drawCards(n + 2, objectName());
            caoren->turnOver();

            if (caoren->getMark("@kuiwei") == 0)
                room->addPlayerMark(caoren, "@kuiwei");
        } else if (caoren->getPhase() == Player::Draw) {
            if (caoren->getMark("@kuiwei") == 0)
                return false;
            room->removePlayerMark(caoren, "@kuiwei");
            int n = getWeaponCount(caoren);
            if (n > 0) {
                LogMessage log;
                log.type = "#KuiweiDiscard";
                log.from = caoren;
                log.arg = QString::number(n);
                log.arg2 = objectName();
                room->sendLog(log);

                room->askForDiscard(caoren, objectName(), n, n, false, true);
            }
        }
        return false;
    }
};

class Yanzheng : public OneCardViewAsSkill
{
public:
    Yanzheng() : OneCardViewAsSkill("yanzheng")
    {
        filter_pattern = ".|.|.|equipped";
        response_or_use = true;
    }

    bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return pattern == "nullification" && player->getHandcardNum() > player->getHp();
    }

    const Card *viewAs(const Card *originalCard) const
    {
        Nullification *ncard = new Nullification(originalCard->getSuit(), originalCard->getNumber());
        ncard->addSubcard(originalCard);
        ncard->setSkillName(objectName());
        return ncard;
    }

    bool isEnabledAtNullification(const ServerPlayer *player) const
    {
        return player->getHandcardNum() > player->getHp() && !player->getEquips().isEmpty();
    }
};

BGMPackage::BGMPackage() : Package("BGM")
{
    General *bgm_zhaoyun = new General(this, "bgm_zhaoyun", "qun", 3); // *SP 001
    bgm_zhaoyun->addSkill("longdan");
    bgm_zhaoyun->addSkill(new Chongzhen);

    General *bgm_diaochan = new General(this, "bgm_diaochan", "qun", 3, false); // *SP 002
    bgm_diaochan->addSkill(new Lihun);
    bgm_diaochan->addSkill("biyue");

    General *bgm_caoren = new General(this, "bgm_caoren", "wei"); // *SP 003
    bgm_caoren->addSkill(new Kuiwei);
    bgm_caoren->addSkill(new Yanzheng);

    addMetaObject<LihunCard>();
}

ADD_PACKAGE(BGM)
