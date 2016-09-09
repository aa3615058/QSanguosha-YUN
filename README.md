# QSanguosha-YUN
Based on <a href="https://github.com/Mogara/QSanguosha-v2">Qsanguosha-V2-20150926</a>.    
Compeletly new general package <b>YUN</b> and <b>YUNEX</b> and a few optimization to Qsanguosha.    
I have designed these generals for my friends. We have a good time when playing Sanguosha.    
I develop this project also for commemoration to my college life.

## YUN package
<table>
  <tbody>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN006huaibeibeiskin6.jpg" width="320" height="450"/></td>
      <td>
        <b>红颜</b>：锁定技，你的黑桃牌视为红桃牌。<br>
        <b>天成</b>：每当你使用或打出一张手牌时，或你的判定牌生效后，若触发了技能“红颜”，你可以摸一张牌。若此时是你的出牌阶段且你本阶段已发动“天成”的次数为奇数，将“摸一张牌”改为“令本回合你的手牌上限+1”。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN007hanjingskin3.jpg" width="320" height="450"/></td>
      <td>
        <b>连击</b>：你的回合外，每当一名角色对你攻击范围内的其他角色造成伤害后，你可以对受到伤害的角色使用一张【杀】，然后你摸一张牌。<br />
        <b>巧破</b>：每当你受到1点伤害时，你可以交给一名其他角色一张方块牌并将伤害转移之。 
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN008wangcanskin2.jpg" width="320" height="450"/></td>
      <td>
        <b>丝舞</b>：每当你受到1点伤害后， 你可以摸一张牌，然后将一张手牌置于你的武将牌上。称为“丝”。（“丝”背面朝上放置）你可以将“丝”如手牌般使用或打出。<br>
        <b>星灿</b>：出牌阶段对每名角色限一次， 你可以将一张“丝”交给一名角色，然后你摸一张牌。若这名角色不是你，你选择一项：该角色可以立即使用这张牌；或除处于濒死状态时，该角色不能使用或打出手牌直至回合结束。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN009yangwenqiskin1.jpg" width="320" height="450"/></td>
      <td>
        <b>战鬼</b>：出牌阶段，你有以下技能：你本回合使用的首张【杀】可以额外指定至多两名角色为目标。锁定技，你使用【杀】无距离限制，你使用【杀】的所有目标角色需座次连续且至少有一名目标角色与你座次相邻。<br>
        <b>调略</b>：出牌阶段限一次，你可以将一张红色手牌当【调虎离山】使用。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN010xiaosaskin1.jpg" width="320" height="450"/></td>
      <td>
        <b>潇寒</b>：你可以将一张普通【杀】当【雷杀】使用；你对一名角色造成雷电伤害时，若该角色有牌，你可以防止此伤害，改为依次弃置其两张牌；锁定技，你是任何【闪电】造成伤害的来源。<br>
        <b>秘雨</b>：结束阶段，若你已受伤，你可以选择至多X名其他角色，视为这些角色各判定一次【闪电】。X为你已损失体力值的一半（向上取整）。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN011lishuyuskin2.jpg" width="320" height="450"/></td>
      <td>
        <b>影咒</b>：出牌阶段限一次，若你本阶段上一张使用或打出的牌是基本牌或非延时类锦囊牌，你可以将一张手牌当做这张牌使用或打出。<br>
        <b>奇风</b>：每当你对其他角色造成伤害时，你可以令另一名其他角色弃置一张红色手牌，否则将伤害转移之。（转移的伤害不能发动“奇风”）
      </td>
    </tr>
  </tbody>
</table>

## YUNEX package
 
<table>
  <tbody>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUNEX001liyunpeng.jpg" width="320" height="450"/></td>
      <td>
        <b>蓝颜</b>：锁定技，你的回合外，你的性别视为女。<br>
        <b>烈女</b>：每当你受到异性角色造成的一次伤害后，或你对同性角色造成一次伤害后，你可以进行一次判定，若结果为黑色，你将此牌交给一名角色；若结果为红色，你可以弃置一张牌令一名已受伤的角色回复一点体力。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUNEX002huaibeibei.jpg" width="320" height="450"/></td>
      <td>
        <b>红颜</b>：锁定技，你的黑桃牌视为红桃牌。<br>
        <b>亦歌</b>：准备阶段开始时，你可以选择一名其他女性角色，直到你下次触发该技能：你可以选择拥有该角色的一项技能（除主公技、限定技与觉醒技），你可以令该角色拥有技能“红颜”。游戏开始时，若场上没有其他女性角色，你可以失去技能“亦歌”和“兼美”，获得技能“天成”。<br>
        <b>兼美</b>：主公技，每名角色的回合限一次，每当其他角色使用或打出一张手牌时，或其他角色的判定牌生效后，若触发了技能“红颜”，该角色可以令你摸一张牌。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUNEX003hanjing.jpg" width="320" height="450"/></td>
      <td>
        <b>断雁</b>：一名男性角色的准备阶段开始时，你可以交给其一张方块牌，这名角色受到你造成的1点伤害并摸X张牌，X为这名角色与你的距离的一半（向下取整）。<br>
        <b>凭风</b>：锁定技，你的装备区没有牌时，视为你拥有技能“飞影”；你的装备区有牌时，视为你拥有技能“流离”。
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUNEX004xiaosa.jpg" width="320" height="450"/></td>
      <td>
        <b>雷压</b>：你使用的【杀】结算完毕后，若你使用这张【杀】造成了伤害，可以视为你使用一张黑桃花色的【铁索连环】。锁定技，你为伤害来源的【杀】或【决斗】造成的伤害均视为雷电伤害。<br>
        <b>震月</b>：你可以跳过摸牌阶段，出牌阶段和弃牌阶段，选择一项并视为你使用一张【雷杀】：计算距离时-X；或可以额外指定至多X名角色为目标。X为你已损失体力值的一半（向上取整）。
      </td>
    </tr>
  </tbody>
 </table>
 
## YUNSP package
 
 <table>
  <tbody>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN001zhaowenting.jpg" width="320" height="450"/></td>
      <td>
        <b>天秤</b>：在一名角色的判定牌生效前，你可以从牌堆顶翻出一张牌代替之。<br>
        <b>闭月</b>：结束阶段，你可以摸一张牌。<br>
        <b>朝夕</b>：主公技，觉醒技，准备阶段，若你是体力值最小的角色，你加1点体力上限，回复1点体力，然后获得技能“礼赞”。<br>
        <blockquote><b>礼赞</b>：主公技，每名男性角色的回合限一次，其他角色的判定牌为红色时，可以令你摸一张牌。</blockquote>
      </td>
    </tr>
    <tr>
      <td width="340"><img src="https://github.com/aa3615058/YUNCardImages/blob/master/card-output/YUN003liyunpeng.jpg" width="320" height="450"/></td>
      <td>
        <b>仙剑</b>：出牌阶段限一次，每当你使用【杀】时，你可以进行判定，若结果为黑色：你可以额外指定一个目标，若如此做，你可以重复此流程直至判定结果为红色，你获得此牌。<br>
        <b>奇侠</b>：每当一名角色进入濒死状态时，你可以与该角色各摸一张牌。
      </td>
    </tr>
  </tbody>
 </table>
