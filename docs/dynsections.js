function toggleVisibility(e){var n=$(e).attr("id"),i=$("#"+n+"-summary"),s=$("#"+n+"-content"),o=$("#"+n+"-trigger"),t=$(o).attr("src");return s.is(":visible")===!0?(s.hide(),i.show(),$(e).addClass("closed").removeClass("opened"),$(o).attr("src",t.substring(0,t.length-8)+"closed.png")):(s.show(),i.hide(),$(e).removeClass("closed").addClass("opened"),$(o).attr("src",t.substring(0,t.length-10)+"open.png")),!1}function updateStripes(){$("table.directory tr").removeClass("even").filter(":visible:even").addClass("even"),$("table.directory tr").removeClass("odd").filter(":visible:odd").addClass("odd")}function toggleLevel(e){$("table.directory tr").each(function(){var t=this.id.split("_").length-1,n=$("#img"+this.id.substring(3)),s=$("#arr"+this.id.substring(3));t<e+1?(n.removeClass("iconfopen iconfclosed").addClass("iconfopen"),s.html("&#9660;"),$(this).show()):t==e+1?(n.removeClass("iconfclosed iconfopen").addClass("iconfclosed"),s.html("&#9658;"),$(this).show()):$(this).hide()}),updateStripes()}function toggleFolder(e){var t,o,n=$("#row_"+e),i=n.nextAll("tr"),a=new RegExp("^row_"+e+"\\d+_$","i"),s=i.filter(function(){return this.id.match(a)});s.filter(":first").is(":visible")===!0?(t=n.find("span"),t.filter(".iconfopen").removeClass("iconfopen").addClass("iconfclosed"),t.filter(".arrow").html("&#9658;"),i.filter("[id^=row_"+e+"]").hide()):(t=n.find("span"),t.filter(".iconfclosed").removeClass("iconfclosed").addClass("iconfopen"),t.filter(".arrow").html("&#9660;"),o=s.find("span"),o.filter(".iconfopen").removeClass("iconfopen").addClass("iconfclosed"),o.filter(".arrow").html("&#9658;"),s.show()),updateStripes()}function toggleInherit(e){var n=$("tr.inherit."+e),s=$("tr.inherit_header."+e+" img"),t=$(s).attr("src");n.filter(":first").is(":visible")===!0?(n.css("display","none"),$(s).attr("src",t.substring(0,t.length-8)+"closed.png")):(n.css("display","table-row"),$(s).attr("src",t.substring(0,t.length-10)+"open.png"))}var opened=!0,plusImg=["url('plus.svg')","url('../../plus.svg')"],minusImg=["url('minus.svg')","url('../../minus.svg')"];function codefold_toggle_all(e){opened?($("#fold_all").css("background-image",plusImg[e]),$("div[id^=foldopen]").hide(),$("div[id^=foldclosed]").show()):($("#fold_all").css("background-image",minusImg[e]),$("div[id^=foldopen]").show(),$("div[id^=foldclosed]").hide()),opened=!opened}function codefold_toggle(e){$("#foldopen"+e).toggle(),$("#foldclosed"+e).toggle()}function init_codefold(e){$("span[class=lineno]").css({"padding-right":"4px","margin-right":"2px",display:"inline-block",width:"54px",background:"linear-gradient(#808080,#808080) no-repeat 46px/2px 100%"}),$("span[class=lineno]:first").append('<span class="fold" id="fold_all" onclick="javascript:codefold_toggle_all('+e+');" style="background-image:'+minusImg[e]+';"></span>'),$("span[class=lineno]").not(":eq(0)").append('<span class="fold"></span>'),$("div[class=foldopen]").each(function(){var t,n=$(this).attr("id").replace("foldopen",""),s=$(this).attr("data-start"),o=$(this).attr("data-end");$(this).find("span[class=fold]:first").replaceWith(`<span class="fold" onclick="javascript:codefold_toggle('`+n+`');" style="background-image:`+minusImg[e]+';"></span>'),$(this).after('<div id="foldclosed'+n+'" class="foldclosed" style="display:none;"></div>'),t=$(this).children().first().clone(),$(t).removeClass("glow"),s&&$(t).html($(t).html().replace(new RegExp("\\s*"+s+"\\s*$","g"),"")),$(t).find("span[class=fold]").css("background-image",plusImg[e]),$(t).append(" "+s+`<a href="javascript:codefold_toggle('`+n+`')">&#8230;</a>`+o),$("#foldclosed"+n).html(t)})}