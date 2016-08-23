sgs.ai_skill_invoke.lienv = true
sgs.ai_skill_use["@@lienv"] = function(self, prompt)
	local function getLeastValueCard()
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
			self:sortByUseValue(cards)
			for _, c in ipairs(cards) do
				if self:getUseValue(c) < 6 and not self:isValuableCard(c) and not self.player:isJilei(c) then
					if not isCard("Peach", c, self.player) then
						return c:getEffectiveId()
					end
				end
			end
			if offhorse_avail and not self.player:isJilei(self.player:getOffensiveHorse()) then return self.player:getOffensiveHorse():getEffectiveId() end
		end
		return nil
	end

	--筛选卡牌方案1
	local card = getLeastValueCard()
	--筛选卡牌方案2
	if not card then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		card = cards[1]:getEffectiveId()
	end

	--筛选对象
	local arr1 = self:getWoundedFriend(false, true)
	local target = nil

	if #arr1 > 0 and (self:isWeak(arr1[1]) or self:getOverflow() >= 1) and arr1[1]:getHp() < getBestHp(arr1[1]) then target = arr1[1] end
	if self:getOverflow() >= 0 and #arr1 > 0 then
		for _, friend in ipairs(arr1) do
			if not friend:hasSkills("hunzi|longhun") then
				target = friend
				break
			end
		end
	end

	if card and target then
		return "@LienvCard=" .. card .. "->" .. target:objectName()
	else
		return nil
	end
end
sgs.dynamic_value.benefit.LienvCard = true

sgs.ai_skill_playerchosen.jianmei = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, target in ipairs(targets) do
		if self:isFriend(target) and target:isAlive() then
			return target
		end
	end
	return nil
end
