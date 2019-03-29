function SetMinHeight() {
    var subMenuHeight;
    var pageContentsHeight;

    subMenuHeight = document.getElementById("subMenu").offsetHeight;

    pageContentsHeight = document.getElementById("pageContents").offsetHeight;

    if (subMenuHeight > pageContentsHeight) {
        document.getElementById("pageContents").style.minHeight = subMenuHeight + "px";
    }
}
