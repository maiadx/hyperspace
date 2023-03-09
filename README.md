This project is archived and not actively being developed anymore, sorry :(


It's codebase may be repurposed and brought back in the future for a few ideas I might have for it, so its being left up for now.

git filter-branch --env-filter 'OLD_EMAIL="43254987+jimdox@users.noreply.github.com" NEW_NAME="Maia Doxtader" NEW_EMAIL="maiadoxtader@proton.me" if [ "$GIT_COMMITTER_EMAIL" = "$OLD_EMAIL" ] then export GIT_COMMITTER_NAME="$NEW_NAME" export GIT_COMMITTER_EMAIL="$NEW_EMAIL" fi if [ "$GIT_AUTHOR_EMAIL" = "$OLD_EMAIL" ] then export GIT_AUTHOR_NAME="$NEW_NAME" export GIT_AUTHOR_EMAIL="$NEW_EMAIL" fi' --tag-name-filter cat -- --branches --tags; git push --force --tags origin 'refs/heads/*'