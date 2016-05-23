#include "ol.h"
#include "sp.h"
#include "client.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "engine.h"
#include "maneuvering.h"
#include "json.h"
#include "settings.h"
#include "clientplayer.h"
#include "util.h"
#include "wrapped-card.h"
#include "room.h"
#include "roomthread.h"
#include "clientstruct.h"


class AocaiViewAsSkill : public ZeroCardViewAsSkill
{
public:
    AocaiViewAsSkill() : ZeroCardViewAsSkill("aocai")
    {
    }

    bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (player->getPhase() != Player::NotActive || player->hasFlag("Global_AocaiFailed")) return false;
        if (pattern == "slash")
            return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
        else if (pattern == "peach")
            return player->getMark("Global_PreventPeach") == 0;
        else if (pattern.contains("analeptic"))
            return true;
        return false;
    }

    const Card *viewAs() const
    {
        AocaiCard *aocai_card = new AocaiCard;
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "peach+analeptic" && Self->getMark("Global_PreventPeach") > 0)
            pattern = "analeptic";
        aocai_card->setUserString(pattern);
        return aocai_card;
    }
};

class Aocai : public TriggerSkill
{
public:
    Aocai() : TriggerSkill("aocai")
    {
        events << CardAsked;
        view_as_skill = new AocaiViewAsSkill;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QString pattern = data.toStringList().first();
        if (player->getPhase() == Player::NotActive
            && (pattern == "slash" || pattern == "jink")
            && room->askForSkillInvoke(player, objectName(), data)) {
            QList<int> ids = room->getNCards(2, false);
            QList<int> enabled, disabled;
            foreach (int id, ids) {
                if (Sanguosha->getCard(id)->objectName().contains(pattern))
                    enabled << id;
                else
                    disabled << id;
            }
            int id = Aocai::view(room, player, ids, enabled, disabled);
            if (id != -1) {
                const Card *card = Sanguosha->getCard(id);
                room->provide(card);
                return true;
            }
        }
        return false;
    }

    static int view(Room *room, ServerPlayer *player, QList<int> &ids, QList<int> &enabled, QList<int> &disabled)
    {
        int result = -1, index = -1;
        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = player;
        log.card_str = IntList2StringList(ids).join("+");
        room->sendLog(log, player);

        room->broadcastSkillInvoke("aocai");
        room->notifySkillInvoked(player, "aocai");
        if (enabled.isEmpty()) {
            JsonArray arg;
            arg << "." << false << JsonUtils::toJsonArray(ids);
            room->doNotify(player, QSanProtocol::S_COMMAND_SHOW_ALL_CARDS, arg);
        } else {
            room->fillAG(ids, player, disabled);
            int id = room->askForAG(player, enabled, true, "aocai");
            if (id != -1) {
                index = ids.indexOf(id);
                ids.removeOne(id);
                result = id;
            }
            room->clearAG(player);
        }

        QList<int> &drawPile = room->getDrawPile();
        for (int i = ids.length() - 1; i >= 0; i--)
            drawPile.prepend(ids.at(i));
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, QVariant(drawPile.length()));
        if (result == -1)
            room->setPlayerFlag(player, "Global_AocaiFailed");
        else {
            LogMessage log;
            log.type = "#AocaiUse";
            log.from = player;
            log.arg = "aocai";
            log.arg2 = QString("CAPITAL(%1)").arg(index + 1);
            room->sendLog(log);
        }
        return result;
    }
};


AocaiCard::AocaiCard()
{
}

bool AocaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool AocaiCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
        return true;

    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFixed();
}

bool AocaiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetsFeasible(targets, Self);
}

const Card *AocaiCard::validateInResponse(ServerPlayer *user) const
{
    Room *room = user->getRoom();
    QList<int> ids = room->getNCards(2, false);
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    QList<int> enabled, disabled;
    foreach (int id, ids) {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "aocai";
    room->sendLog(log);

    int id = Aocai::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

const Card *AocaiCard::validate(CardUseStruct &cardUse) const
{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *user = cardUse.from;
    Room *room = user->getRoom();
    QList<int> ids = room->getNCards(2, false);
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    QList<int> enabled, disabled;
    foreach (int id, ids) {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "aocai";
    room->sendLog(log);

    int id = Aocai::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

DuwuCard::DuwuCard()
{
    mute = true;
}

bool DuwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty() || qMax(0, to_select->getHp()) != subcardsLength())
        return false;

    if (Self->getWeapon() && subcards.contains(Self->getWeapon()->getId())) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        int distance_fix = weapon->getRange() - Self->getAttackRange(false);
        if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
            distance_fix += 1;
        return Self->inMyAttackRange(to_select, distance_fix);
    } else if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId())) {
        return Self->inMyAttackRange(to_select, 1);
    } else
        return Self->inMyAttackRange(to_select);
}

void DuwuCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();

    if (subcards.length() <= 1)
        room->broadcastSkillInvoke("duwu", 2);
    else
        room->broadcastSkillInvoke("duwu", 1);

    room->damage(DamageStruct("duwu", effect.from, effect.to));
}

class DuwuViewAsSkill : public ViewAsSkill
{
public:
    DuwuViewAsSkill() : ViewAsSkill("duwu")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->canDiscard(player, "he") && !player->hasFlag("DuwuEnterDying");
    }

    bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return !Self->isJilei(to_select);
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        DuwuCard *duwu = new DuwuCard;
        if (!cards.isEmpty())
            duwu->addSubcards(cards);
        return duwu;
    }
};

class Duwu : public TriggerSkill
{
public:
    Duwu() : TriggerSkill("duwu")
    {
        events << QuitDying;
        view_as_skill = new DuwuViewAsSkill;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage && dying.damage->getReason() == "duwu" && !dying.damage->chain && !dying.damage->transfer) {
            ServerPlayer *from = dying.damage->from;
            if (from && from->isAlive()) {
                room->setPlayerFlag(from, "DuwuEnterDying");
                room->loseHp(from, 1);
            }
        }
        return false;
    }
};

class Biluan : public PhaseChangeSkill
{
public:
    Biluan() : PhaseChangeSkill("biluan")
    {

    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        switch (target->getPhase()) {
        case (Player::Draw) :
            if (PhaseChangeSkill::triggerable(target) && target->askForSkillInvoke(this)) {
                target->getRoom()->setPlayerMark(target, "biluan", 1);
                return true;
            }
            break;
        case (Player::RoundStart) :
            target->getRoom()->setPlayerMark(target, "biluan", 0);
            break;
        default:

            break;
        }

        return false;
    }
};

class BiluanDist : public DistanceSkill
{
public:
    BiluanDist() : DistanceSkill("#biluan-dist")
    {

    }

    int getCorrect(const Player *, const Player *to) const
    {
        if (to->getMark("biluan") == 1) {
            QList<const Player *> sib = to->getAliveSiblings();
            sib << to;
            QSet<QString> kingdoms;
            foreach (const Player *p, sib)
                kingdoms.insert(p->getKingdom());

            return kingdoms.count();
        }

        return 0;
    }
};

class Lixia : public PhaseChangeSkill
{
public:
    Lixia() : PhaseChangeSkill("lixia")
    {

    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        Room *r = target->getRoom();
        switch (target->getPhase()) {
        case (Player::Finish) : {
            QList<ServerPlayer *> misterious1s = r->getOtherPlayers(target);
            foreach (ServerPlayer *misterious1, misterious1s) {
                if (TriggerSkill::triggerable(misterious1) && !target->inMyAttackRange(misterious1) && misterious1->askForSkillInvoke(this, QVariant::fromValue(target))) {
                    misterious1->drawCards(1, objectName());
                    r->addPlayerMark(misterious1, "lixia", 1);
                }
            }
        }
            break;
        case (Player::RoundStart) :
            target->getRoom()->setPlayerMark(target, "lixia", 0);
            break;
        default:

            break;
        }

        return false;
    }
};

class LixiaDist : public DistanceSkill
{
public:
    LixiaDist() : DistanceSkill("#lixia-dist")
    {

    }

    int getCorrect(const Player *, const Player *to) const
    {
        return -to->getMark("lixia");
    }
};

class Yishe : public TriggerSkill
{
public:
    Yishe() : TriggerSkill("yishe")
    {
        events << EventPhaseStart << CardsMoveOneTime;
        frequency = Frequent;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseStart) {
            if (player->getPile("rice").isEmpty() && player->getPhase() == Player::Finish) {
                if (player->askForSkillInvoke(this)) {
                    player->drawCards(2, objectName());
                    if (!player->isNude()) {
                        const Card *dummy = NULL;
                        if (player->getCardCount(true) <= 2) {
                            DummyCard *dummy2 = new DummyCard;
                            dummy2->addSubcards(player->getCards("he"));
                            dummy2->deleteLater();
                            dummy = dummy2;
                        } else
                            dummy = room->askForExchange(player, objectName(), 2, 2, true, "@yishe");

                        player->addToPile("rice", dummy);
                        delete dummy;
                    }
                }
            }
        } else {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from == player && move.from_pile_names.contains("rice") && move.from->getPile("rice").isEmpty())
                room->recover(player, RecoverStruct(player));
        }

        return false;
    }
};

BushiCard::BushiCard()
{
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void BushiCard::onUse(Room *, const CardUseStruct &card_use) const
{
    // log

    card_use.from->obtainCard(this);
}

class BushiVS : public OneCardViewAsSkill
{
public:
    BushiVS() : OneCardViewAsSkill("bushi")
    {
        response_pattern = "@@bushi";
        filter_pattern = ".|.|.|rice,%rice";
        expand_pile = "rice,%rice";
    }

    const Card *viewAs(const Card *card) const
    {
        BushiCard *bs = new BushiCard;
        bs->addSubcard(card);
        return bs;
    }
};

class Bushi : public TriggerSkill
{
public:
    Bushi() : TriggerSkill("bushi")
    {
        events << Damage << Damaged;
        view_as_skill = new BushiVS;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player->getPile("rice").isEmpty())
            return false;

        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *p = damage.to;

        if (damage.from->isDead() || damage.to->isDead())
            return false;

        for (int i = 0; i < damage.damage; ++i) {
            if (!room->askForUseCard(p, "@@bushi", "@bushi", -1, Card::MethodNone))
                break;

            if (p->isDead() || player->getPile("rice").isEmpty())
                break;
        }

        return false;
    }
};

MidaoCard::MidaoCard()
{
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void MidaoCard::onUse(Room *, const CardUseStruct &card_use) const
{
    card_use.from->tag["midao"] = subcards.first();
}

class MidaoVS : public OneCardViewAsSkill
{
public:
    MidaoVS() : OneCardViewAsSkill("midao")
    {
        response_pattern = "@@midao";
        filter_pattern = ".|.|.|rice";
        expand_pile = "rice";
    }

    const Card *viewAs(const Card *card) const
    {
        MidaoCard *bs = new MidaoCard;
        bs->addSubcard(card);
        return bs;
    }
};

class Midao : public RetrialSkill
{
public:
    Midao() : RetrialSkill("midao", false)
    {
        view_as_skill = new MidaoVS;
    }

    const Card *onRetrial(ServerPlayer *player, JudgeStruct *judge) const
    {
        if (player->getPile("rice").isEmpty())
            return NULL;

        QStringList prompt_list;
        prompt_list << "@midao-card" << judge->who->objectName()
            << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");

        player->tag.remove("midao");
        player->tag["judgeData"] = QVariant::fromValue(judge);
        Room *room = player->getRoom();
        bool invoke = room->askForUseCard(player, "@@midao", prompt, -1, Card::MethodNone);
        player->tag.remove("judgeData");
        if (invoke && player->tag.contains("midao")) {
            int id = player->tag.value("midao", player->getPile("rice").first()).toInt();
            return Sanguosha->getCard(id);
        }

        return NULL;
    }
};

class FengpoRecord : public TriggerSkill
{
public:
    FengpoRecord() : TriggerSkill("#fengpo-record")
    {
        events << EventPhaseChanging << PreCardUsed << CardResponded;
        global = true;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseChanging) {
            if (data.value<PhaseChangeStruct>().to == Player::Play)
                room->setPlayerFlag(player, "-fengporec");
        } else {
            if (player->getPhase() != Player::Play)
                return false;

            const Card *c = NULL;
            if (triggerEvent == PreCardUsed)
                c = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_isUse)
                    c = resp.m_card;
            }

            if (c == NULL || c->isKindOf("SkillCard"))
                return false;
            
            if (triggerEvent == PreCardUsed && !player->hasFlag("fengporec"))
                room->setCardFlag(c, "fengporecc");

            room->setPlayerFlag(player, "fengporec");
        }

        return false;
    }
};

class Fengpo : public TriggerSkill
{
public:
    Fengpo() : TriggerSkill("fengpo")
    {
        events << TargetSpecified << DamageCaused << CardFinished;
    }

    bool trigger(TriggerEvent e, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (e == TargetSpecified) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.to.length() != 1) return false;
            if (use.to.first()->isKongcheng()) return false;
            if (use.card == NULL) return false;
            if (!use.card->isKindOf("Slash") && !use.card->isKindOf("Duel")) return false;
            if (!use.card->hasFlag("fengporecc")) return false;
            QStringList choices;
            int n = 0;
            foreach (const Card *card, use.to.first()->getHandcards())
                if (card->getSuit() == Card::Diamond)
                    ++n;
            if (n > 0) choices << "drawCards";
            choices << "addDamage" << "cancel";
            QString choice = room->askForChoice(player, objectName(), choices.join("+"));
            if (choice == "drawCards")
                player->drawCards(n);
            else if (choice == "addDamage")
                player->tag["fengpoaddDamage" + use.card->toString()] = n;
        } else if (e == DamageCaused) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card == NULL || damage.from == NULL)
                return false;
            if (damage.from->tag.contains("fengpoaddDamage" + damage.card->toString()) && (damage.card->isKindOf("Slash") || damage.card->isKindOf("Duel"))) {
                damage.damage += damage.from->tag.value("fengpoaddDamage" + damage.card->toString()).toInt();
                data = QVariant::fromValue(damage);
                damage.from->tag.remove("fengpoaddDamage" + damage.card->toString());
            }
        } else if (e == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.to.length() != 1) return false;
            if (use.to.first()->isKongcheng()) return false;
            if (!use.card->isKindOf("Slash") || !use.card->isKindOf("Duel")) return false;
            if (player->tag.contains("fengpoaddDamage" + use.card->toString()))
                player->tag.remove("fengpoaddDamage" + use.card->toString());
        }
        return false;
    }
};


OLPackage::OLPackage()
    : Package("OL")
{
    General *zhugeke = new General(this, "zhugeke", "wu", 3); // OL 002
    zhugeke->addSkill(new Aocai);
    zhugeke->addSkill(new Duwu);

    /*General *ol_masu = new General(this, "ol_masu", "shu", 3);
    ol_masu->addSkill(new Sanyao);
    ol_masu->addSkill(new Zhiman);

    General *ol_yujin = new General(this, "ol_yujin", "wei");
    ol_yujin->addSkill(new Jieyue);

    General *ol_liubiao = new General(this, "ol_liubiao", "qun", 3);
    ol_liubiao->addSkill(new OlZishou);
    ol_liubiao->addSkill(new OlZishouProhibit);
    ol_liubiao->addSkill("zongshi");

    /*General *ol_xusheng = new General(this, "ol_xusheng", "wu");
    ol_xusheng->addSkill(new OlPojun);
    ol_xusheng->addSkill(new FakeMoveSkill("olpojun"));
    related_skills.insertMulti("olpojun", "#olpojun-fake-move");*/

    General *shixie = new General(this, "shixie", "qun", 3);
    shixie->addSkill(new Biluan);
    shixie->addSkill(new BiluanDist);
    shixie->addSkill(new Lixia);
    shixie->addSkill(new LixiaDist);
    related_skills.insertMulti("biluan", "#biluan-dist");
    related_skills.insertMulti("lixia", "#lixia-dist");

    General *zhanglu = new General(this, "zhanglu", "qun", 3);
    zhanglu->addSkill(new Yishe);
    zhanglu->addSkill(new Bushi);
    zhanglu->addSkill(new Midao);

    General *mayl = new General(this, "mayunlu", "shu", 4, false);
    mayl->addSkill("mashu");
    mayl->addSkill(new Fengpo);
    mayl->addSkill(new FengpoRecord);
    related_skills.insertMulti("fengpo", "#fengpo-record");

    /*General *zhangjiao = new General(this, "ol_zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new OlLeiji);
    zhangjiao->addSkill("guidao");
    zhangjiao->addSkill("huangtian");*/

    addMetaObject<AocaiCard>();
    addMetaObject<DuwuCard>();
    addMetaObject<MidaoCard>();
    addMetaObject<BushiCard>();
}

ADD_PACKAGE(OL)
