sgs.ai_skill_invoke.tiancheng = true
sgs.tiancheng_suit_value = {
	spade = 3.9
}

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