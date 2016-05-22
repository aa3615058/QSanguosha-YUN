#ifndef _NOSTALGIA_H
#define _NOSTALGIA_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "standard-skillcards.h"

class NostalgiaPackage : public Package
{
    Q_OBJECT

public:
    NostalgiaPackage();
};

class NostalStandardPackage : public Package
{
    Q_OBJECT

public:
    NostalStandardPackage();
};

class NosTuxiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosTuxiCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class NosRendeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosRendeCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosKurouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosKurouCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosFanjianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosFanjianCard();
    void onEffect(const CardEffectStruct &effect) const;
};

class NosLijianCard : public LijianCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosLijianCard();
};

class QingnangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class NosYiji : public MasochismSkill
{
public:
    NosYiji();
    void onDamaged(ServerPlayer *target, const DamageStruct &damage) const;

protected:
    int n;
};

#endif

