sgs.ai_skill_invoke.lienv = true

sgs.ai_skill_playerchosen.jianmei = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, target in ipairs(targets) do
		if self:isFriend(target) and target:isAlive() then
			return target
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.jianmei = -50