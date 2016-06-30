sgs.ai_skill_invoke.tiancheng = true
sgs.tiancheng_suit_value = {
	spade = 3.9
}

function sgs.ai_cardneed.lianji(to, card, self)
	local cards = to:getHandcards()
	local has_weapon = to:getWeapon() and not to:getWeapon():isKindOf("Crossbow")
	local slash_num = 0
	for _, c in sgs.qlist(cards) do
		local flag=string.format("%s_%s_%s","visible",self.room:getCurrent():objectName(),to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if c:isKindOf("Weapon") and not c:isKindOf("Crossbow") then
				has_weapon=true
			end
			if c:isKindOf("Slash") then slash_num = slash_num +1 end
		end
	end

	if not has_weapon then
		return card:isKindOf("Weapon") and not card:isKindOf("Crossbow")
	else
		return to:hasWeapon("spear")  or card:isKindOf("Slash")
	end
end

sgs.lianji_keep_value = {
	Peach = 5,
	Analeptic = 4.8,
	Jink = 5.2,
	Slash = 5.4,
	ThunderSlash = 5.5,
	FireSlash = 5.6,
	DoubleSword = 5.2,
	Halberd = 4.9,
	Axe = 4.9,
	Fan = 4.9,
	Spear = 4.9,
	KylinBow = 4.8,
	QinggangSword = 4.7,
	Blade = 4.7,
	IceSword = 4.5,
	Crossbow = 4,
	Duel = 4,
	DefensiveHorse = 4,
	OffensiveHorse = 5,
}

sgs.qiaopo_suit_value = {
	diamond = 4.2
}

function sgs.ai_cardneed.qiaopo(to, card, self)
	return card:getSuit() == sgs.Card_Diamond and getKnownCard(to, self.player, "diamond", false, "he") < 2
end

sgs.ai_skill_use["@@qiaopo"] = function(self, data)
	local function getLeastValueDiamondCard(reverse = false)
		local offhorse_avail, weapon_avail
		for _, enemy in ipairs(self.enemies) do
			if self:canAttack(enemy, self.player) then
				if not offhorse_avail and self.player:getOffensiveHorse() and self.player:distanceTo(enemy, 1) <= self.player:getAttackRange() then
					offhorse_avail = true
				end
				if not weapon_avail and self.player:getWeapon() and self.player:distanceTo(enemy) == 1 then
					weapon_avail = true
				end
			end
			if offhorse_avail and weapon_avail then break end
		end
		if self:needToThrowArmor() then return self.player:getArmor():getEffectiveId() end
		if self.player:getPhase() > sgs.Player_Play then
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if self:getKeepValue(c) < 8 and not self.player:isJilei(c) and not self:isValuableCard(c) then return c:getEffectiveId() end
			end
			if offhorse_avail and not self.player:isJilei(self.player:getOffensiveHorse()) then return self.player:getOffensiveHorse():getEffectiveId() end
			if weapon_avail and not self.player:isJilei(self.player:getWeapon()) and self:evaluateWeapon(self.player:getWeapon()) < 5 then return self.player:getWeapon():getEffectiveId() end
		else
			local cards = sgs.QList2Table(self.player:getHandcards())
			local diamondCards = {}
			for _, c in ipairs(cards) do
				if c.getSuit() == sgs.Card_Diamond then
					table.insert(diamondCards, c)
				end
			end
			self:sortByUseValue(diamondCards)
			if not reverse then
				for _, c in ipairs(diamondCards) do
					if self:getUseValue(c) < 6 and not self:isValuableCard(c) then
						if not isCard("Peach", c, self.player) then
							return c:getEffectiveId()
						end
					end
				end
			else
				return diamondCards[#diamondCards]
			end
			if offhorse_avail then return self.player:getOffensiveHorse():getEffectiveId() end
		end
		return nil
	end


	-- if not method then method = sgs.Card_MethodDiscard end
	-- local friend_lost_hp = 10
	-- local friend_hp = 0
	-- local card_id
	-- local target
	-- local cant_use_skill
	-- local dmg

	if data == "@qiaopo-card" then
		dmg = self.player:getTag("QiaopoDamage"):toDamage()
	else
		dmg = data
	end

	-- if not dmg then self.room:writeToConsole(debug.traceback()) return "." end

	-- local cards = self.player:getCards("h")
	-- cards = sgs.QList2Table(cards)
	-- self:sortByUseValue(cards, true)
	-- for _, card in ipairs(cards) do
	-- 	if not self.player:isCardLimited(card, method) and card:getSuit() == sgs.Card_Heart and not card:isKindOf("Peach") then
	-- 		card_id = card:getId()
	-- 		break
	-- 	end
	-- end
	-- if not card_id then return "." end

	self:sort(self.enemies, "hp")

	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 1 and enemy:isAlive() and enemy:getLostHp() + dmg.damage < 3) then
			if (enemy:getHandcardNum() <= 2 or enemy:hasSkills("guose|leiji|ganglie|enyuan|qingguo|wuyan|kongcheng") or enemy:containsTrick("indulgence"))
				and self:canAttack(enemy, dmg.from or self.room:getCurrent(), dmg.nature)
				and not (dmg.card and dmg.card:getTypeId() == sgs.Card_TypeTrick and enemy:hasSkill("wuyan")) then
				return "@TianxiangCard=" .. card_id .. "->" .. enemy:objectName()
			end
		end
	end

	-- for _, friend in ipairs(self.friends_noself) do
	-- 	if (friend:getLostHp() + dmg.damage > 1 and friend:isAlive()) then
	-- 		if friend:isChained() and dmg.nature ~= sgs.DamageStruct_Normal and not self:isGoodChainTarget(friend, dmg.from, dmg.nature, dmg.damage, dmg.card) then
	-- 		elseif friend:getHp() >= 2 and dmg.damage < 2
	-- 				and (friend:hasSkills("yiji|buqu|nosbuqu|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu")
	-- 					or self:getDamagedEffects(friend, dmg.from or self.room:getCurrent())
	-- 					or self:needToLoseHp(friend)
	-- 					or (friend:getHandcardNum() < 3 and (friend:hasSkill("nosrende") or (friend:hasSkill("rende") and not friend:hasUsed("RendeCard"))))) then
	-- 			return "@TianxiangCard=" .. card_id .. "->" .. friend:objectName()
	-- 			elseif dmg.card and dmg.card:getTypeId() == sgs.Card_TypeTrick and friend:hasSkill("wuyan") and friend:getLostHp() > 1 then
	-- 				return "@TianxiangCard=" .. card_id .. "->" .. friend:objectName()
	-- 		elseif hasBuquEffect(friend) then return "@TianxiangCard=" .. card_id .. "->" .. friend:objectName() end
	-- 	end
	-- end

	-- for _, enemy in ipairs(self.enemies) do
	-- 	if (enemy:getLostHp() <= 1 or dmg.damage > 1) and enemy:isAlive() and enemy:getLostHp() + dmg.damage < 4 then
	-- 		if (enemy:getHandcardNum() <= 2)
	-- 			or enemy:containsTrick("indulgence") or enemy:hasSkills("guose|leiji|vsganglie|ganglie|enyuan|qingguo|wuyan|kongcheng")
	-- 			and self:canAttack(enemy, (dmg.from or self.room:getCurrent()), dmg.nature)
	-- 			and not (dmg.card and dmg.card:getTypeId() == sgs.Card_TypeTrick and enemy:hasSkill("wuyan")) then
	-- 			return "@TianxiangCard=" .. card_id .. "->" .. enemy:objectName() end
	-- 	end
	-- end

	-- for i = #self.enemies, 1, -1 do
	-- 	local enemy = self.enemies[i]
	-- 	if not enemy:isWounded() and not self:hasSkills(sgs.masochism_skill, enemy) and enemy:isAlive()
	-- 		and self:canAttack(enemy, dmg.from or self.room:getCurrent(), dmg.nature)
	-- 		and (not (dmg.card and dmg.card:getTypeId() == sgs.Card_TypeTrick and enemy:hasSkill("wuyan") and enemy:getLostHp() > 0) or self:isWeak()) then
	-- 		return "@TianxiangCard=" .. card_id .. "->" .. enemy:objectName()
	-- 	end
	-- end

	-- return "."
end

-- sgs.ai_card_intention.QiaopoCard = function(self, card, from, tos)
-- 	local to = tos[1]
-- 	if self:getDamagedEffects(to) or self:needToLoseHp(to) then return end
-- 	local intention = 10
-- 	if hasBuquEffect(to) then intention = 0
-- 	elseif (to:getHp() >= 2 and to:hasSkills("yiji|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu"))
-- 		or (to:getHandcardNum() < 3 and (to:hasSkill("nosrende") or (to:hasSkill("rende") and not to:hasUsed("RendeCard")))) then
-- 		intention = 0
-- 	end
-- 	sgs.updateIntention(from, to, intention)
-- end

function sgs.ai_slash_prohibit.qiaopo(self, from, to)
	if from:hasSkill("jueqing") or (from:hasSkill("nosqianxi") and from:distanceTo(to) == 1) then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	if self:isFriend(to, from) then return false end
	return self:cantbeHurt(to, from)
end

--“潇寒（雷杀）”跟“符箓”的AI是一样的
sgs.ai_view_as.xiaohan_thunder_slash = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_PlaceSpecial and card:getClassName() == "Slash" and not card:hasFlag("using") then
		return ("thunder_slash:xiaohan_thunder_slash[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.ai_skill_invoke.xiaohan_thunder_slash = function(self, data)
	local use = data:toCardUse()
	for _, player in sgs.qlist(use.to) do
		if self:isEnemy(player) and self:damageIsEffective(player, sgs.DamageStruct_Thunder) and sgs.isGoodTarget(player, self.enemies, self) then
			return true
		end
	end
	return false
end

local xiaohan_thunder_slash_skill = {}
xiaohan_thunder_slash_skill.name = "xiaohan_thunder_slash"
table.insert(sgs.ai_skills, xiaohan_thunder_slash_skill)
xiaohan_thunder_slash_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)

	local slash
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getClassName() == "Slash" then
			slash = card
			break
		end
	end

	if not slash then return nil end
	local dummy_use = { to = sgs.SPlayerList(), isDummy = true }
	self:useCardThunderSlash(slash, dummy_use)
	if dummy_use.card and dummy_use.to:length() > 0 then
		local use = sgs.CardUseStruct()
		use.from = self.player
		use.to = dummy_use.to
		use.card = slash
		local data = sgs.QVariant()
		data:setValue(use)
		if not sgs.ai_skill_invoke.xiaohan_thunder_slash(self, data) then return nil end
	else return nil end

	if slash then
		local suit = slash:getSuitString()
		local number = slash:getNumberString()
		local card_id = slash:getEffectiveId()
		local card_str = ("thunder_slash:xiaohan_thunder_slash[%s:%s]=%d"):format(suit, number, card_id)
		local mySlash = sgs.Card_Parse(card_str)

		assert(mySlash)
		return mySlash
	end
end

--“潇寒（寒冰剑）”跟“寒冰剑”的AI是一样的
sgs.ai_skill_invoke.xiaohan_ice_sword = function(self, data)
	local damage = data:toDamage()
	local target = damage.to

	if self:isFriend(target) then
		if self:getDamagedEffects(target, self.players, true) or self:needToLoseHp(target, self.player, true) then return false
		elseif target:isChained() and self:isGoodChainTarget(target, self.player, nil, nil, damage.card) then return false
		elseif self:isWeak(target) or damage.damage > 1 then return true
		elseif target:getLostHp() < 1 then return false end
		return true
	else
		if self:isWeak(target) then return false end
		if damage.damage > 1 or self:hasHeavySlashDamage(self.player, damage.card, target) then return false end
		if target:hasSkill("lirang") and #self:getFriendsNoself(target) > 0 then return false end
		if target:getArmor() and self:evaluateArmor(target:getArmor(), target) > 3 and not (target:hasArmorEffect("silver_lion") and target:isWounded()) then return true end
		local num = target:getHandcardNum()
		if self.player:hasSkill("tieji") or self:canLiegong(target, self.player) then return false end
		if target:hasSkills("tuntian+zaoxian") and target:getPhase() == sgs.Player_NotActive then return false end
		if self:hasSkills(sgs.need_kongcheng, target) then return false end
		if target:getCards("he"):length()<4 and target:getCards("he"):length()>1 then return true end
		return false
	end
end

sgs.ai_skill_use["@@miyu"] = function(self, prompt)
	local stand_value = 100
	local getCmpValue = function(enemy)
		local value = 0
		local player = self.player
		if not self:damageIsEffective(enemy, sgs.DamageStruct_Thunder, player) then return 99 end
		if enemy:hasSkill("hongyan") then 
			if enemy:hasSkill("tiancheng") or enemy:hasSkill("tiandu") or enemy:hasSKill("qianxun") then return 100
			else return 99 end
		end
		if enemy:hasSkill("tianxiang") and enemy:getCards("h"):length() >= 2 then value = value + 90 end
		if enemy:hasSkill("tiandu") then value = value + 80 end
		if enemy:hasSkill("qianxun") then value = value + 80 end
		if enemy:hasSkill("qiaopo") then value = value + 40 end
		if self:cantbeHurt(enemy, player) or self:objectiveLevel(enemy) < 3
			or (enemy:isChained() and not self:isGoodChainTarget(enemy, player, sgs.DamageStruct_Thunder)) then return 100 end
		if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value + 50 end
		if enemy:hasArmorEffect("silver_lion") then value = value + 20 end
		if enemy:hasSkills(sgs.exclusive_skill) then value = value + 10 end
		if enemy:hasSkills(sgs.masochism_skill) then value = value + 5 end
		if enemy:isChained() and self:isGoodChainTarget(enemy, player, sgs.DamageStruct_Thunder) and #(self:getChainedEnemies(player)) > 1 then value = value - 25 end
		if enemy:isLord() then value = value - 5 end
		value = value + enemy:getHp() + sgs.getDefenseSlash(enemy, self) * 0.01
		return value
	end

	local cmp = function(a, b)
		return getCmpValue(a) < getCmpValue(b)
	end

	local getMiYuTargetCount = function() 
		return math.ceil(self.player:getLostHp() / 2)
	end

	local enemies = self.enemies
	table.sort(enemies, cmp)
	local targetTable = {}
	for _,enemy in ipairs(enemies) do
		if getCmpValue(enemy) < stand_value then 
			table.insert(targetTable, enemy:objectName())
			if #targetTable >= getMiYuTargetCount() then
				break
			end
		end
	end
	return "@MiyuCard=.->" .. table.concat(targetTable, "+")
end
