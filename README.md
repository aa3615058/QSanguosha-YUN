# QSanguosha-YUN
Based on <a href="https://github.com/Mogara/QSanguosha-v2">Qsanguosha-V2-20150926</a>.    
Compeletly new general package <b>YUN</b> and <b>YUNEX</b> and some optimization to Qsanguosha.    
I designed, tested, printed and then programmed these Sanguosha general cards for my college classmates as graduation gifts. We had a good time when playing Sanguosha.    
This project is also a commemoration of my college life.

## YUN package
<table>
  <tbody>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN006huaibeibei.jpg" width="320" height="450"/></td>
      <td>
        <b>红颜</b>：锁定技，你的♠牌视为♥牌。<br>
        <b>天成</b>：当你使用或打出手牌时，或你的判定牌生效后，若触发了技能〖红颜〗，你可以摸一张牌。若此时是你的出牌阶段，且你于此阶段内发动过此技能，你须弃置一张牌，然后本回合你的手牌上限+1。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN007hanjing.jpg" width="320" height="450"/></td>
      <td>
        <b>连击</b>：你的回合外，当一名角色对你攻击范围内的其他角色造成伤害后，你可以对受到伤害的角色使用一张【杀】，然后你摸一张牌。<br />
        <b>巧破</b>：当你受到 1 点伤害时，你可以交给一名其他角色一张♦牌并此伤害转移给该角色。 
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN008wangcanskin2.jpg" width="320" height="450"/></td>
      <td>
        <b>丝舞</b>：当你受到 1 点伤害后，你可以摸一张牌，然后将一张手牌扣置于武将牌上，称为“丝”。你可以将“丝”如手牌般使用或打出。当你使用或打出一张牌时，若此牌不是你的手牌，你可以摸一张牌。<br>
        <b>星灿</b>：出牌阶段结束时，你可以将一张手牌置于牌堆顶，然后跳过弃牌阶段。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN009yangwenqiskin2.jpg" width="320" height="450"/></td>
      <td>
        <b>战鬼</b>：出牌阶段，你使用的第一张【杀】可以多选择至多两名角色为目标。锁定技，你使用【杀】无距离限制，你使用【杀】的所有目标角色须座次连续且至少有一名目标角色与你座次相邻。<br>
        <b>调略</b>：你可以将一张红色手牌当【调虎离山】使用。当你使用的【调虎离山】结算结束后，你可以摸一张牌。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN010xiaosaskin2.jpg" width="320" height="450"/></td>
      <td>
        <b>潇寒</b>：你可以将一张普通【杀】当【雷杀】使用；当你对一名角色造成雷电伤害时，若该角色有牌，你可以防止此伤害，改为依次弃置其两张牌；锁定技，你是任何【闪电】造成伤害的来源。<br>
        <b>秘雨</b>：结束阶段，若你已受伤，你可以选择至多 X 名其他角色，视为这些角色各判定一次【闪电】。X 为你已损失体力值的一半（向上取整）。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN011lishuyu.jpg" width="320" height="450"/></td>
      <td>
        <b>影咒</b>：出牌阶段，若你于此阶段内使用的上一张牌是非此技能转化的基本牌或普通锦囊牌，你可以将与此牌颜色不同的一张手牌当此牌使用。<br>
        <b>奇风</b>：当你对其他角色造成非此技能转移的伤害时，你可以弃置一张♠手牌并选择另一名角色，你将此伤害转移给该角色，然后其摸 X 张牌，X 为该角色已损失的体力值。
      </td>
    </tr>
  </tbody>
</table>

## YUNEX package
 
<table>
  <tbody>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUNEX001EXliyunpeng.jpg" width="320" height="450"/></td>
      <td>
        <b>蓝颜</b>：锁定技，你的回合外，你的性别视为女。<br>
        <b>烈女</b>：当你造成或受到伤害后，你可以进行判定，若结果为：黑色，你获得判定牌；红色，你可以弃置一张牌令一名已受伤的同性角色回复 1 点体力。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUNEX002EXhuaibeibeifullskin.jpg" width="320" height="450"/></td>
      <td>
        <b>红颜</b>：锁定技，你的♠牌视为♥牌。<br>
        <b>亦歌</b>：回合开始时，你可以选择一名其他女性角色，直到你下次发动此技能之前：你可以获得该角色的一个技能（主公技、限定技、觉醒技除外）；你可以令该角色获得技能〖红颜〗。游戏开始时，你可以选择失去技能〖亦歌〗和〖兼美〗，获得技能〖天成〗。<br>
        <b>兼美</b>：主公技，每名角色的回合限一次，当其他角色使用或打出手牌时，或其他角色的判定牌生效后，若触发了技能〖红颜〗，该角色可以令你摸一张牌。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUNEX003EXhanjingfullskin.jpg" width="320" height="450"/></td>
      <td>
        <b>断雁</b>：其他角色的出牌阶段开始时，你可以交给其一张♦牌，该角色受到你造成的 1 点伤害，若该角色为男性，其摸 X 张牌，X 为该角色与你距离的一半（向下取整）。<br>
        <b>凭风</b>：锁定技， 若你的装备区没有牌，你视为拥有技能〖飞影〗；若你的装备区有牌，你视为拥有技能〖流离〗。
      </td>
    </tr>
  </tbody>
</table>
