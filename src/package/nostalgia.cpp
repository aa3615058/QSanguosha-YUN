#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "nostalgia.h"
#include "yjcm.h"
#include "yjcm2013.h"
#include "settings.h"
#include "wind.h"
#include "clientplayer.h"
#include "clientstruct.h"
#include "room.h"
#include "roomthread.h"
// old stantard generals

class NosJianxiong : public MasochismSkill
{
public:
    NosJianxiong() : MasochismSkill("nosjianxiong")
    {
    }

    void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const
    {
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        if (!card) return;

        QList<int> ids;
        if (card->isVirtualCard())
            ids = card->getSubcards();
        else
            ids << card->getEffectiveId();

        if (ids.isEmpty()) return;
        foreach (int id, ids) {
            if (room->getCardPlace(id) != Player::PlaceTable) return;
        }
        QVariant data = QVariant::fromValue(damage);
        if (room->askForSkillInvoke(caocao, objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            caocao->obtainCard(card);
        }
    }
};

class NosFankui : public MasochismSkill
{
public:
    NosFankui() : MasochismSkill("nosfankui")
    {
    }

    void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const
    {
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if (from && !from->isNude() && room->askForSkillInvoke(simayi, "nosfankui", data)) {
            room->broadcastSkillInvoke(objectName());
            int card_id = room->askForCardChosen(simayi, from, "he", "nosfankui");
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, simayi->objectName());
            room->obtainCard(simayi, Sanguosha->getCard(card_id),
                reason, room->getCardPlace(card_id) != Player::PlaceHand);
        }
    }
};

class NosGuicai : public RetrialSkill
{
public:
    NosGuicai() : RetrialSkill("nosguicai")
    {

    }

    const Card *onRetrial(ServerPlayer *player, JudgeStruct *judge) const
    {
        if (player->isKongcheng())
            return NULL;

        QStringList prompt_list;
        prompt_list << "@nosguicai-card" << judge->who->objectName()
            << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");

        Room *room = player->getRoom();
        const Card *card = room->askForCard(player, ".", prompt, QVariant::fromValue(judge), Card::MethodResponse, judge->who, true);
        if (card)
            room->broadcastSkillInvoke(objectName());

        return card;
    }
};

class NosGanglie : public MasochismSkill
{
public:
    NosGanglie() : MasochismSkill("nosganglie")
    {
    }

    void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const
    {
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant data = QVariant::fromValue(damage);

        if (room->askForSkillInvoke(xiahou, "nosganglie", data)) {
            room->broadcastSkillInvoke("nosganglie");

            JudgeStruct judge;
            judge.pattern = ".|heart";
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if (!from || from->isDead()) return;
            if (judge.isGood()) {
                if (from->getHandcardNum() < 2 || !room->askForDiscard(from, objectName(), 2, 2, true))
                    room->damage(DamageStruct(objectName(), xiahou, from));
            }
        }
    }
};

NosTuxiCard::NosTuxiCard()
{
}

bool NosTuxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.length() >= 2 || to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void NosTuxiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    if (effect.from->isAlive() && !effect.to->isKongcheng()) {
        int card_id = room->askForCardChosen(effect.from, effect.to, "h", "tuxi");
        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
        room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, false);
    }
}

class NosTuxiViewAsSkill : public ZeroCardViewAsSkill
{
public:
    NosTuxiViewAsSkill() : ZeroCardViewAsSkill("nostuxi")
    {
        response_pattern = "@@nostuxi";
    }

    const Card *viewAs() const
    {
        return new NosTuxiCard;
    }
};

class NosTuxi : public PhaseChangeSkill
{
public:
    NosTuxi() : PhaseChangeSkill("nostuxi")
    {
        view_as_skill = new NosTuxiViewAsSkill;
    }

    bool onPhaseChange(ServerPlayer *zhangliao) const
    {
        if (zhangliao->getPhase() == Player::Draw) {
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach (ServerPlayer *player, other_players) {
                if (!player->isKongcheng()) {
                    can_invoke = true;
                    break;
                }
            }

            if (can_invoke && room->askForUseCard(zhangliao, "@@nostuxi", "@nostuxi-card"))
                return true;
        }

        return false;
    }
};

class NosLuoyiBuff : public TriggerSkill
{
public:
    NosLuoyiBuff() : TriggerSkill("#nosluoyi")
    {
        events << DamageCaused;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->hasFlag("nosluoyi") && target->isAlive();
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *xuchu, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.chain || damage.transfer || !damage.by_user) return false;
        const Card *reason = damage.card;
        if (reason && (reason->isKindOf("Slash") || reason->isKindOf("Duel"))) {
            LogMessage log;
            log.type = "#LuoyiBuff";
            log.from = xuchu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class NosLuoyi : public DrawCardsSkill
{
public:
    NosLuoyi() : DrawCardsSkill("nosluoyi")
    {
    }

    int getDrawNum(ServerPlayer *xuchu, int n) const
    {
        Room *room = xuchu->getRoom();
        if (room->askForSkillInvoke(xuchu, objectName())) {
            room->broadcastSkillInvoke(objectName());
            xuchu->setFlags(objectName());
            return n - 1;
        } else
            return n;
    }
};

NosYiji::NosYiji() : MasochismSkill("nosyiji")
{
    frequency = Frequent;
    n = 2;
}

void NosYiji::onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const
{
    Room *room = guojia->getRoom();
    int x = damage.damage;
    for (int i = 0; i < x; i++) {
        if (!guojia->isAlive() || !room->askForSkillInvoke(guojia, objectName()))
            return;
        room->broadcastSkillInvoke("nosyiji");

        QList<ServerPlayer *> _guojia;
        _guojia.append(guojia);
        QList<int> yiji_cards = room->getNCards(n, false);

        CardsMoveStruct move(yiji_cards, NULL, guojia, Player::PlaceTable, Player::PlaceHand,
            CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString()));
        QList<CardsMoveStruct> moves;
        moves.append(move);
        room->notifyMoveCards(true, moves, false, _guojia);
        room->notifyMoveCards(false, moves, false, _guojia);

        QList<int> origin_yiji = yiji_cards;
        while (room->askForYiji(guojia, yiji_cards, objectName(), true, false, true, -1, room->getAlivePlayers())) {
            CardsMoveStruct move(QList<int>(), guojia, NULL, Player::PlaceHand, Player::PlaceTable,
                CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString()));
            foreach (int id, origin_yiji) {
                if (room->getCardPlace(id) != Player::DrawPile) {
                    move.card_ids << id;
                    yiji_cards.removeOne(id);
                }
            }
            origin_yiji = yiji_cards;
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->notifyMoveCards(true, moves, false, _guojia);
            room->notifyMoveCards(false, moves, false, _guojia);
            if (!guojia->isAlive())
                return;
        }

        if (!yiji_cards.isEmpty()) {
            CardsMoveStruct move(yiji_cards, guojia, NULL, Player::PlaceHand, Player::PlaceTable,
                CardMoveReason(CardMoveReason::S_REASON_PREVIEW, guojia->objectName(), objectName(), QString()));
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->notifyMoveCards(true, moves, false, _guojia);
            room->notifyMoveCards(false, moves, false, _guojia);

            DummyCard *dummy = new DummyCard(yiji_cards);
            guojia->obtainCard(dummy, false);
            delete dummy;
        }
    }
}

NosRendeCard::NosRendeCard()
{
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void NosRendeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.first();

    QDateTime dtbefore = source->tag.value("nosrende", QDateTime(QDate::currentDate(), QTime(0, 0, 0))).toDateTime();
    QDateTime dtafter = QDateTime::currentDateTime();

    if (dtbefore.secsTo(dtafter) > 3 * Config.AIDelay / 1000)
        room->broadcastSkillInvoke("rende");

    source->tag["nosrende"] = QDateTime::currentDateTime();

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "nosrende", QString());
    room->obtainCard(target, this, reason, false);

    int old_value = source->getMark("nosrende");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "nosrende", new_value);

    if (old_value < 2 && new_value >= 2)
        room->recover(source, RecoverStruct(source));
}

class NosRendeViewAsSkill : public ViewAsSkill
{
public:
    NosRendeViewAsSkill() : ViewAsSkill("nosrende")
    {
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (ServerInfo.GameMode == "04_1v3" && selected.length() + Self->getMark("nosrende") >= 2)
            return false;
        else
            return !to_select->isEquipped();
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        if (ServerInfo.GameMode == "04_1v3" && player->getMark("nosrende") >= 2)
            return false;
        return !player->isKongcheng();
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.isEmpty())
            return NULL;

        NosRendeCard *rende_card = new NosRendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class NosRende : public TriggerSkill
{
public:
    NosRende() : TriggerSkill("nosrende")
    {
        events << EventPhaseChanging;
        view_as_skill = new NosRendeViewAsSkill;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->getMark("nosrende") > 0;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;
        room->setPlayerMark(player, "nosrende", 0);
        return false;
    }
};

class NosTieji : public TriggerSkill
{
public:
    NosTieji() : TriggerSkill("nostieji")
    {
        events << TargetSpecified;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash"))
            return false;
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();
        int index = 0;
        foreach (ServerPlayer *p, use.to) {
            if (!player->isAlive()) break;
            if (player->askForSkillInvoke(this, QVariant::fromValue(p))) {
                room->broadcastSkillInvoke(objectName());

                p->setFlags("NosTiejiTarget"); // For AI

                JudgeStruct judge;
                judge.pattern = ".|red";
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                try {
                    room->judge(judge);
                }
                catch (TriggerEvent triggerEvent) {
                    if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                        p->setFlags("-NosTiejiTarget");
                    throw triggerEvent;
                }

                if (judge.isGood()) {
                    LogMessage log;
                    log.type = "#NoJink";
                    log.from = p;
                    room->sendLog(log);
                    jink_list.replace(index, QVariant(0));
                }

                p->setFlags("-NosTiejiTarget");
            }
            index++;
        }
        player->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        return false;
    }
};

class NosJizhi : public TriggerSkill
{
public:
    NosJizhi() : TriggerSkill("nosjizhi")
    {
        frequency = Frequent;
        events << CardUsed;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *yueying, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isNDTrick() && room->askForSkillInvoke(yueying, objectName())) {
            room->broadcastSkillInvoke("jizhi");
            yueying->drawCards(1, objectName());
        }

        return false;
    }
};

class NosQicai : public TargetModSkill
{
public:
    NosQicai() : TargetModSkill("nosqicai")
    {
        pattern = "TrickCard";
    }

    int getDistanceLimit(const Player *from, const Card *) const
    {
        if (from->hasSkill(this))
            return 1000;
        else
            return 0;
    }
};

NosKurouCard::NosKurouCard()
{
    target_fixed = true;
}

void NosKurouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->loseHp(source);
    if (source->isAlive())
        room->drawCards(source, 2, "noskurou");
}

class NosKurou : public ZeroCardViewAsSkill
{
public:
    NosKurou() : ZeroCardViewAsSkill("noskurou")
    {
    }

    const Card *viewAs() const
    {
        return new NosKurouCard;
    }
};

class NosYingzi : public DrawCardsSkill
{
public:
    NosYingzi() : DrawCardsSkill("nosyingzi")
    {
        frequency = Frequent;
    }

    int getDrawNum(ServerPlayer *zhouyu, int n) const
    {
        Room *room = zhouyu->getRoom();
        if (room->askForSkillInvoke(zhouyu, objectName())) {
            room->broadcastSkillInvoke("nosyingzi");
            return n + 1;
        } else
            return n;
    }
};

NosFanjianCard::NosFanjianCard()
{
}

void NosFanjianCard::onEffect(const CardEffectStruct &effect) const
{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = zhouyu->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, "nosfanjian");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(card);
    room->showCard(target, card_id);

    if (card->getSuit() != suit)
        room->damage(DamageStruct("nosfanjian", zhouyu, target));
}

class NosFanjian : public ZeroCardViewAsSkill
{
public:
    NosFanjian() : ZeroCardViewAsSkill("nosfanjian")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->isKongcheng() && !player->hasUsed("NosFanjianCard");
    }

    const Card *viewAs() const
    {
        return new NosFanjianCard;
    }
};

class NosGuose : public OneCardViewAsSkill
{
public:
    NosGuose() : OneCardViewAsSkill("nosguose")
    {
        filter_pattern = ".|diamond";
        response_or_use = true;
    }

    const Card *viewAs(const Card *originalCard) const
    {
        Indulgence *indulgence = new Indulgence(originalCard->getSuit(), originalCard->getNumber());
        indulgence->addSubcard(originalCard->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class NosQianxun : public ProhibitSkill
{
public:
    NosQianxun() : ProhibitSkill("nosqianxun")
    {
    }

    bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const
    {
        return to->hasSkill(this) && (card->isKindOf("Snatch") || card->isKindOf("Indulgence"));
    }
};

class NosLianying : public TriggerSkill
{
public:
    NosLianying() : TriggerSkill("noslianying")
    {
        events << CardsMoveOneTime;
        frequency = Frequent;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *luxun, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == luxun && move.from_places.contains(Player::PlaceHand) && move.is_last_handcard) {
            if (room->askForSkillInvoke(luxun, objectName(), data)) {
                room->broadcastSkillInvoke(objectName());
                luxun->drawCards(1, objectName());
            }
        }
        return false;
    }
};

QingnangCard::QingnangCard()
{
}

bool QingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    return targets.value(0, Self)->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.value(0, source);
    room->cardEffect(this, source, target);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const
{
    effect.to->getRoom()->recover(effect.to, RecoverStruct(effect.from));
}

class Qingnang : public OneCardViewAsSkill
{
public:
    Qingnang() : OneCardViewAsSkill("qingnang")
    {
        filter_pattern = ".|.|.|hand!";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->canDiscard(player, "h") && !player->hasUsed("QingnangCard");
    }

    const Card *viewAs(const Card *originalCard) const
    {
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(originalCard->getId());
        return qingnang_card;
    }
};

NosLijianCard::NosLijianCard() : LijianCard(false)
{
}

class NosLijian : public OneCardViewAsSkill
{
public:
    NosLijian() : OneCardViewAsSkill("noslijian")
    {
        filter_pattern = ".!";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->getAliveSiblings().length() > 1
            && player->canDiscard(player, "he") && !player->hasUsed("NosLijianCard");
    }

    const Card *viewAs(const Card *originalCard) const
    {
        NosLijianCard *lijian_card = new NosLijianCard;
        lijian_card->addSubcard(originalCard->getId());
        return lijian_card;
    }

    int getEffectIndex(const ServerPlayer *, const Card *card) const
    {
        return card->isKindOf("Duel") ? 0 : -1;
    }
};

NostalStandardPackage::NostalStandardPackage()
    : Package("nostal_standard")
{
    General *nos_caocao = new General(this, "nos_caocao$", "wei");
    nos_caocao->addSkill(new NosJianxiong);
    nos_caocao->addSkill("hujia");

    General *nos_simayi = new General(this, "nos_simayi", "wei", 3);
    nos_simayi->addSkill(new NosFankui);
    nos_simayi->addSkill(new NosGuicai);

    General *nos_xiahoudun = new General(this, "nos_xiahoudun", "wei");
    nos_xiahoudun->addSkill(new NosGanglie);

    General *nos_zhangliao = new General(this, "nos_zhangliao", "wei");
    nos_zhangliao->addSkill(new NosTuxi);

    General *nos_xuchu = new General(this, "nos_xuchu", "wei");
    nos_xuchu->addSkill(new NosLuoyi);
    nos_xuchu->addSkill(new NosLuoyiBuff);
    related_skills.insertMulti("nosluoyi", "#nosluoyi");

    General *nos_guojia = new General(this, "nos_guojia", "wei", 3);
    nos_guojia->addSkill("tiandu");
    nos_guojia->addSkill(new NosYiji);

    General *nos_liubei = new General(this, "nos_liubei$", "shu");
    nos_liubei->addSkill(new NosRende);
    nos_liubei->addSkill("jijiang");

    General *nos_guanyu = new General(this, "nos_guanyu", "shu");
    nos_guanyu->addSkill("wusheng");

    General *nos_zhangfei = new General(this, "nos_zhangfei", "shu");
    nos_zhangfei->addSkill("paoxiao");

    General *nos_zhaoyun = new General(this, "nos_zhaoyun", "shu");
    nos_zhaoyun->addSkill("longdan");

    General *nos_machao = new General(this, "nos_machao", "shu");
    nos_machao->addSkill("mashu");
    nos_machao->addSkill(new NosTieji);

    General *nos_huangyueying = new General(this, "nos_huangyueying", "shu", 3, false);
    nos_huangyueying->addSkill(new NosJizhi);
    nos_huangyueying->addSkill(new NosQicai);

    General *nos_ganning = new General(this, "nos_ganning", "wu");
    nos_ganning->addSkill("qixi");

    General *nos_lvmeng = new General(this, "nos_lvmeng", "wu");
    nos_lvmeng->addSkill("keji");

    General *nos_huanggai = new General(this, "nos_huanggai", "wu");
    nos_huanggai->addSkill(new NosKurou);

    General *nos_zhouyu = new General(this, "nos_zhouyu", "wu", 3);
    nos_zhouyu->addSkill(new NosYingzi);
    nos_zhouyu->addSkill(new NosFanjian);

    General *nos_daqiao = new General(this, "nos_daqiao", "wu", 3, false);
    nos_daqiao->addSkill(new NosGuose);
    nos_daqiao->addSkill("liuli");

    General *nos_luxun = new General(this, "nos_luxun", "wu", 3);
    nos_luxun->addSkill(new NosQianxun);
    nos_luxun->addSkill(new NosLianying);

    General *nos_huatuo = new General(this, "nos_huatuo", "qun", 3);
    nos_huatuo->addSkill(new Qingnang);
    nos_huatuo->addSkill("jijiu");

    General *nos_lvbu = new General(this, "nos_lvbu", "qun");
    nos_lvbu->addSkill("wushuang");

    General *nos_diaochan = new General(this, "nos_diaochan", "qun", 3, false);
    nos_diaochan->addSkill(new NosLijian);
    nos_diaochan->addSkill("biyue");

    addMetaObject<NosTuxiCard>();
    addMetaObject<NosRendeCard>();
    addMetaObject<NosKurouCard>();
    addMetaObject<NosFanjianCard>();
    addMetaObject<NosLijianCard>();
    addMetaObject<QingnangCard>();
}

ADD_PACKAGE(NostalStandard)

NostalgiaPackage::NostalgiaPackage()
    : Package("nostalgia")
{
    type = CardPack;
}
ADD_PACKAGE(Nostalgia)
