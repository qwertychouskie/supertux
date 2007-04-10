//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "infoblock.hpp"
#include "game_session.hpp"
#include "resources.hpp"
#include "sprite/sprite_manager.hpp"
#include "object_factory.hpp"
#include "lisp/lisp.hpp"
#include "sector.hpp"
#include "log.hpp"
#include "object/player.hpp"

InfoBlock::InfoBlock(const lisp::Lisp& lisp)
  : Block(sprite_manager->create("images/objects/bonus_block/infoblock.sprite")), shown_pct(0), dest_pct(0)
{
  Vector pos;
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  bbox.set_pos(pos);

  if(!lisp.get("message", message)) {
    log_warning << "No message in InfoBlock" << std::endl;
  }
  //stopped = false;
  //ringing = new AmbientSound(get_pos(), 0.5, 300, 1, "sounds/phone.wav");
  //Sector::current()->add_object(ringing);
  infoBox.reset(new InfoBox(message));
}

InfoBlock::~InfoBlock()
{
}

void
InfoBlock::hit(Player& )
{
  start_bounce();

  //if (!stopped) {
  //  ringing->remove_me();
  //  stopped = true;
  //}

  if (dest_pct != 1) {

    // first hide all other InfoBlocks' messages in same sector
    Sector* parent = Sector::current();
    if (!parent) return;
    for (Sector::GameObjects::iterator i = parent->gameobjects.begin(); i != parent->gameobjects.end(); i++) {
      InfoBlock* block = dynamic_cast<InfoBlock*>(*i);
      if (!block) continue;
      if (block != this) block->hide_message();
    }

    // show our message
    show_message();

  } else {
    hide_message();
  }
}

Player*
InfoBlock::get_nearest_player()
{
  // FIXME: does not really return nearest player

  std::vector<Player*> players = Sector::current()->get_players();
  for (std::vector<Player*>::iterator playerIter = players.begin(); playerIter != players.end(); ++playerIter) {
    Player* player = *playerIter;
    return player;
  }

  return 0;
}

void
InfoBlock::update(float delta)
{
  if (delta == 0) return;

  // hide message if player is too far away
  if (dest_pct > 0) {
    Player* player = get_nearest_player();
    if (player) {
      Vector p1 = this->get_pos() + (this->get_bbox().p2 - this->get_bbox().p1) / 2;
      Vector p2 = player->get_pos() + (player->get_bbox().p2 - player->get_bbox().p1) / 2;
      Vector dist = (p2 - p1);
      float d = dist.norm();
      if (d > 128) dest_pct = 0;
    }
  }

  // handle soft fade-in and fade-out
  if (shown_pct != dest_pct) {
    if (dest_pct > shown_pct) shown_pct = std::min(shown_pct + 2*delta, dest_pct);
    if (dest_pct < shown_pct) shown_pct = std::max(shown_pct - 2*delta, dest_pct);
  }
}

void
InfoBlock::draw(DrawingContext& context)
{
  Block::draw(context);

  if (shown_pct > 0) {
    context.push_transform();
    context.set_translation(Vector(0, 0));
    context.set_alpha(shown_pct);
    infoBox->draw(context);
    context.pop_transform();
  }
}

void
InfoBlock::show_message()
{
  dest_pct = 1;
}

void
InfoBlock::hide_message()
{
  dest_pct = 0;
}

IMPLEMENT_FACTORY(InfoBlock, "infoblock")
